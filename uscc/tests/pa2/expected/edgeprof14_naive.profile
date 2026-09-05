=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 500000
while.cond -> while.end : 1
while.body -> while.cond1 : 500000
while.cond1 -> while.body3 : 8975731
while.cond1 -> while.end4 : 500000
while.body3 -> if.then : 499999
while.body3 -> if.else : 8475732
while.end4 -> while.cond : 500000
if.then -> if.end : 499999
if.end -> while.cond1 : 8975731
if.else -> if.then12 : 4192515
if.else -> if.else14 : 4283217
if.then12 -> if.end13 : 4192515
if.end13 -> if.end : 8475732
if.else14 -> if.end13 : 4283217
END_PROFILE
