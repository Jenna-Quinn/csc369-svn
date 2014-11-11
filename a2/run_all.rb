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

file = File.new('tables.txt', 'a')
file.write((x = "run_all.rb started at #{Time.now}\n") + '-' * (x.length - 1) + "\n\n")

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

  TRACES.each do |trace|
    [50, 100, 150, 200].each do |m|
      command = "cat #{trace} | ./sim -m #{m} -a #{alg}"
      puts "Running command: #{command}\n"
      rows << [trace, m] + `#{command}`.split
    end

    # Add separator between traces
    rows << :separator
  end

  table.rows = rows

  # Write table to tables.txt
  file.write table.to_s + "\n\n"

end

file.close
