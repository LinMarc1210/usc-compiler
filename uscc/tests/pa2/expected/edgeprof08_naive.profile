=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200
while.cond -> while.end : 1
while.body -> while.cond1 : 200
while.cond1 -> while.body3 : 40000
while.cond1 -> while.end4 : 200
while.body3 -> while.cond5 : 40000
while.end4 -> while.cond : 200
while.cond5 -> while.body7 : 8000000
while.cond5 -> while.end8 : 40000
while.body7 -> if.then : 3980000
while.body7 -> if.else : 4020000
while.end8 -> while.cond1 : 40000
if.then -> if.then22 : 1313400
if.then -> if.else24 : 2666600
if.end -> while.cond5 : 8000000
if.else -> if.then14 : 2666600
if.else -> if.else16 : 1353400
if.then14 -> if.end15 : 2666600
if.end15 -> if.end : 4020000
if.else16 -> if.end15 : 1353400
if.then22 -> if.end23 : 1313400
if.end23 -> if.end : 3980000
if.else24 -> if.end23 : 2666600
END_PROFILE
