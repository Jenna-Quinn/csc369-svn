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
  './make.trace'
]

# Use valgrind to create make.trace
puts 'Creating \'make.trace\'...'
unless system 'valgrind --tool=lackey --trace-mem=yes make > make.trace 2>&1'
  puts 'Failed to create \'make.trace\''
  exit!
end

headings = [
    'Hit count',
    'Miss count',
    'Total references',
    'Hit rate',
    'Miss rate'
]

TRACES.each do |trace|
  command = "cat #{trace} | ./sim -m 50 -a fifo"
  puts "\nRunning command: #{command}\n"
  `#{command}`.split.zip(headings) do |value, heading|
    p 'heading', heading
    p 'value', value
  end

  # table = Terminal::Table.new
  # table.title = command
  # table.headings = headings
  # table.rows = rows
  # table.style = {:width => 40}
end
