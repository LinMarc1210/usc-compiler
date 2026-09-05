=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 3000
while.cond -> while.end : 1
while.body -> if.then : 1500
while.body -> if.else : 1500
if.then -> while.cond6 : 1500
if.end -> if.then14 : 3
if.end -> if.end15 : 2997
if.else -> while.cond1 : 1500
while.cond1 -> while.body3 : 4500000
while.cond1 -> while.end4 : 1500
while.body3 -> while.cond1 : 4500000
while.end4 -> if.end : 1500
while.cond6 -> while.body8 : 4500000
while.cond6 -> while.end9 : 1500
while.body8 -> while.cond6 : 4500000
while.end9 -> if.end : 1500
if.then14 -> if.end15 : 3
if.end15 -> while.cond : 3000
END_PROFILE
