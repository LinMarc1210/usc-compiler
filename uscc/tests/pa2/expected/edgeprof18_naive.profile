=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 100000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 0
if.then -> if.end : 100000000
if.end -> if.then8 : 0
if.end -> if.else10 : 100000000
if.else -> if.end : 0
if.then8 -> if.end9 : 0
if.end9 -> if.then19 : 0
if.end9 -> if.else21 : 100000000
if.else10 -> if.end9 : 100000000
if.then19 -> if.end20 : 0
if.end20 -> if.then30 : 0
if.end20 -> if.else32 : 100000000
if.else21 -> if.end20 : 100000000
if.then30 -> if.end31 : 0
if.end31 -> if.then38 : 379505
if.end31 -> if.end39 : 99620495
if.else32 -> if.end31 : 100000000
if.then38 -> if.end39 : 379505
if.end39 -> while.cond : 100000000
END_PROFILE
