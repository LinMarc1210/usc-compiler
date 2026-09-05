=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.then6 : 50000000
if.then -> if.else8 : 50000000
if.end -> while.cond : 200000000
if.else -> if.end : 100000000
if.then6 -> if.then14 : 25000000
if.then6 -> if.else16 : 25000000
if.end7 -> if.end : 100000000
if.else8 -> if.end7 : 50000000
if.then14 -> if.then21 : 8333333
if.then14 -> if.else23 : 16666667
if.end15 -> if.end7 : 50000000
if.else16 -> if.end15 : 25000000
if.then21 -> if.end22 : 8333333
if.end22 -> if.end15 : 25000000
if.else23 -> if.end22 : 16666667
END_PROFILE
