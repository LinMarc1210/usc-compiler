=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.end : 100000000
if.then -> while.cond : 100000000
if.end -> if.then6 : 33333333
if.end -> if.end7 : 66666667
if.then6 -> while.cond : 33333333
if.end7 -> if.then11 : 13333333
if.end7 -> if.end12 : 53333334
if.then11 -> while.cond : 13333333
if.end12 -> if.then16 : 7619047
if.end12 -> if.end17 : 45714287
if.then16 -> while.cond : 7619047
if.end17 -> if.then21 : 4155844
if.end17 -> if.end22 : 41558443
if.then21 -> while.cond : 4155844
if.end22 -> while.cond : 41558443
END_PROFILE
