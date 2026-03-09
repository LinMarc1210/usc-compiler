=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 15000
while.cond -> while.end : 1
while.body -> while.cond1 : 15000
while.cond1 -> while.body3 : 225000000
while.cond1 -> while.end4 : 15000
while.body3 -> if.then : 15000
while.body3 -> if.else : 224985000
while.end4 -> while.cond : 15000
if.then -> if.end : 15000
if.end -> while.cond1 : 225000000
if.else -> if.then6 : 112492500
if.else -> if.else8 : 112492500
if.then6 -> if.end7 : 112492500
if.end7 -> if.end : 224985000
if.else8 -> if.end7 : 112492500
END_PROFILE
