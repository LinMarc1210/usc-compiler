=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 120
while.cond -> while.end : 1
while.body -> while.cond1 : 120
while.cond1 -> while.body3 : 14400
while.cond1 -> while.end4 : 120
while.body3 -> while.cond5 : 14400
while.end4 -> while.cond : 120
while.cond5 -> while.body7 : 1728000
while.cond5 -> while.end8 : 14400
while.body7 -> while.cond9 : 1728000
while.end8 -> while.cond1 : 14400
while.cond9 -> while.body11 : 207360000
while.cond9 -> while.end12 : 1728000
while.body11 -> if.then : 207355000
while.body11 -> if.end : 5000
while.end12 -> while.cond5 : 1728000
if.then -> if.end : 207355000
if.end -> while.cond9 : 207360000
END_PROFILE
