=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 15000
while.cond -> while.end : 1
while.body -> while.cond1 : 15000
while.cond1 -> while.body3 : 225000000
while.cond1 -> while.end4 : 15000
while.body3 -> if.then : 112492500
while.body3 -> if.else : 112507500
while.end4 -> while.cond : 15000
if.then -> if.end : 112492500
if.end -> if.then18 : 2838031
if.end -> if.end19 : 222161969
if.else -> if.then9 : 15000
if.else -> if.else11 : 112492500
if.then9 -> if.end10 : 15000
if.end10 -> if.end : 112507500
if.else11 -> if.end10 : 112492500
if.then18 -> if.end19 : 2838031
if.end19 -> while.cond1 : 225000000
END_PROFILE
