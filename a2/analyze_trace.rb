# analyze_trace.rb
#
# Analyze a program trace generated by +valgrind --tool=lackey --trace-mem=yes <prog>+
# Usage:
#   valgrind --tool=lackey --trace-mem=yes <prog> | ruby analyze_trace.rb <marker_start> <marker_end>

require 'set'

pages = {
    I: Set.new,
    S: Set.new,
    L: Set.new,
    M: Set.new,
}
memory_accesses = {
  before: 0,
  after: 0,
  between: 0
}

if ARGV.size < 2
  puts 'Usage: valgrind --tool=lackey --trace-mem=yes <prog> | ruby analyze_trace.rb <marker_start> <marker_end>'
  exit!
end
marker_start, marker_end = ARGV.pop(2).map { |s| s.hex }

ops = ARGF.each_line.lazy.map { |line|
  line.match /\s?([ISLM])\s{1,2}(.*),/
}.reject { |match|
  match.nil?
}.map { |match|
  kind, address = match.captures
  address = address.hex
  page = address >> 12
  [kind, address, page]
}

location = :before
ops.each { |kind, address, page|
  pages[kind.to_sym] << page

  if location == :before
    if address == marker_start
      location = :between
    else
      memory_accesses[:before] += 1
    end
  elsif location == :between
    memory_accesses[:between] += 1
    location = :after if address == marker_end
  else
    memory_accesses[:after] += 1
  end
}


printf "Unique code pages (I):       : %20d\n", pages[:I].size
printf "Unique data pages (S + L + M): %20d\n", pages[:S].size + pages[:L].size + pages[:M].size
printf "                            S: %20d\n", pages[:S].size
printf "                            L: %20d\n", pages[:L].size
printf "                            M: %20d\n", pages[:M].size
printf "Memory accesses before\n" \
       "and after the markers:         %20d\n", memory_accesses[:before] + memory_accesses[:after]
printf "Memory accesses\n" \
       "in algorithm component:        %20d\n", memory_accesses[:between]
