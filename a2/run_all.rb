# run_all.rb
#
# Run ./sim for all four page replacement algorithms and output a table for each execution
# Usage:
#   ruby run_all.rb

TRACES = [
    '/u/csc369h/fall/pub/a2-traces/simpleloop',
    # '/u/csc369h/fall/pub/a2-traces/matmul-100',
    # '/u/csc369h/fall/pub/a2-traces/blocked-100-25',
    # '<(valgrind --tool=lackey --trace-mem=yes make 2>&1)'
]

TRACES.each do |trace|
  `./sim -f #{trace} -m 150 -a fifo`
end
