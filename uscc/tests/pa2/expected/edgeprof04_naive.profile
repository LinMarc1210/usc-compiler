=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.then3 : 50000000
if.then -> if.else5 : 50000000
if.end -> while.cond : 200000000
if.else -> if.end : 100000000
if.then3 -> if.then8 : 25000000
if.then3 -> if.else10 : 25000000
if.end4 -> if.end : 100000000
if.else5 -> if.end4 : 50000000
if.then8 -> if.then13 : 12500000
if.then8 -> if.else15 : 12500000
if.end9 -> if.end4 : 50000000
if.else10 -> if.end9 : 25000000
if.then13 -> if.then18 : 6250000
if.then13 -> if.else20 : 6250000
if.end14 -> if.end9 : 25000000
if.else15 -> if.end14 : 12500000
if.then18 -> if.end19 : 6250000
if.end19 -> if.end14 : 12500000
if.else20 -> if.end19 : 6250000
END_PROFILE
