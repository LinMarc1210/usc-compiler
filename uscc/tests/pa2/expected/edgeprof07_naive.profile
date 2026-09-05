=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 25000000
while.body -> if.else : 175000000
if.then -> if.end : 25000000
if.end -> while.cond : 200000000
if.else -> if.then4 : 25000000
if.else -> if.else6 : 150000000
if.then4 -> if.end5 : 25000000
if.end5 -> if.end : 175000000
if.else6 -> if.then9 : 25000000
if.else6 -> if.else11 : 125000000
if.then9 -> if.end10 : 25000000
if.end10 -> if.end5 : 150000000
if.else11 -> if.then14 : 25000000
if.else11 -> if.else16 : 100000000
if.then14 -> if.end15 : 25000000
if.end15 -> if.end10 : 125000000
if.else16 -> if.then19 : 25000000
if.else16 -> if.else21 : 75000000
if.then19 -> if.end20 : 25000000
if.end20 -> if.end15 : 100000000
if.else21 -> if.then24 : 25000000
if.else21 -> if.else26 : 50000000
if.then24 -> if.end25 : 25000000
if.end25 -> if.end20 : 75000000
if.else26 -> if.then29 : 25000000
if.else26 -> if.else31 : 25000000
if.then29 -> if.end30 : 25000000
if.end30 -> if.end25 : 50000000
if.else31 -> if.end30 : 25000000
END_PROFILE
