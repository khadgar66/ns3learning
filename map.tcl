# set number of nodes
set opt(nn) 587

# set activity file
set opt(af) $opt(config-path)
append opt(af) /activity.tcl

# set mobility file
set opt(mf) $opt(config-path)
append opt(mf) /mobility.tcl

# set start/stop time
set opt(start) 0.0
set opt(stop) 1000.0

# set floor size
set opt(x) 2050.22
set opt(y) 2459.08
set opt(min-x) 477.23
set opt(min-y) 1560.42

