=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.end : 100000000
if.then -> while.cond : 100000000
if.end -> if.then3 : 33333333
if.end -> if.end4 : 66666667
if.then3 -> while.cond : 33333333
if.end4 -> if.then7 : 13333333
if.end4 -> if.end8 : 53333334
if.then7 -> while.cond : 13333333
if.end8 -> if.then11 : 7619047
if.end8 -> if.end12 : 45714287
if.then11 -> while.cond : 7619047
if.end12 -> if.then15 : 4155844
if.end12 -> if.end16 : 41558443
if.then15 -> while.cond : 4155844
if.end16 -> while.cond : 41558443
END_PROFILE
