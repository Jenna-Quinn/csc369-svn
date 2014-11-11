# run_all.rb
#
# Run ./sim for all four page replacement algorithms and output a table for each execution to a file tables.txt
# Usage:
#   ruby run_all.rb

require 'terminal-table'

TRACES = [
  '/u/csc369h/fall/pub/a2-traces/simpleloop',
  # '/u/csc369h/fall/pub/a2-traces/matmul-100',
  # '/u/csc369h/fall/pub/a2-traces/blocked-100-25',
  # './make.trace'
]

# Use valgrind to create make.trace
# puts "Creating \'make.trace\'...\n"
# command = 'valgrind --tool=lackey --trace-mem=yes make > make.trace 2>&1'
# puts "Running `#{command}`..."
# unless system command
#   puts 'Failed to create \'make.trace\''
#   exit!
# end

headings = [
    'Trace',
    'm',
    'Hits',
    'Misses',
    'Total references',
    'Hit rate',
    'Miss rate'
]

%w(fifo lru clock opt).each do |alg|
  # Create a table per algorithm
  table = Terminal::Table.new
  table.title = alg
  table.headings = headings
  table.style = {width: 120}

  rows = []

  TRACES.product([50, 100, 150, 200]).each do |trace, m|
    command = "cat #{trace} | ./sim -m #{m} -a #{alg}"
    puts "\nRunning command: #{command}\n"
    rows << [trace, m] + `#{command}`.split
  end

  table.rows = rows

  puts table

end
