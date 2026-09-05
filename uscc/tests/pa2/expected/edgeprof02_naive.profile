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
if.else -> if.then2 : 25000000
if.else -> if.else4 : 150000000
if.then2 -> if.end3 : 25000000
if.end3 -> if.end : 175000000
if.else4 -> if.then6 : 25000000
if.else4 -> if.else8 : 125000000
if.then6 -> if.end7 : 25000000
if.end7 -> if.end3 : 150000000
if.else8 -> if.then10 : 25000000
if.else8 -> if.else12 : 100000000
if.then10 -> if.end11 : 25000000
if.end11 -> if.end7 : 125000000
if.else12 -> if.then14 : 25000000
if.else12 -> if.else16 : 75000000
if.then14 -> if.end15 : 25000000
if.end15 -> if.end11 : 100000000
if.else16 -> if.then18 : 25000000
if.else16 -> if.else20 : 50000000
if.then18 -> if.end19 : 25000000
if.end19 -> if.end15 : 75000000
if.else20 -> if.then22 : 25000000
if.else20 -> if.else24 : 25000000
if.then22 -> if.end23 : 25000000
if.end23 -> if.end19 : 50000000
if.else24 -> if.end23 : 25000000
END_PROFILE
