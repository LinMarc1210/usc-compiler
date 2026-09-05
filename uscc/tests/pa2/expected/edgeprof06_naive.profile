=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 2000000
while.body -> if.end : 198000000
if.then -> if.end : 2000000
if.end -> if.then2 : 2000000
if.end -> if.end3 : 198000000
if.then2 -> if.end3 : 2000000
if.end3 -> if.then6 : 2000000
if.end3 -> if.end7 : 198000000
if.then6 -> if.end7 : 2000000
if.end7 -> if.then10 : 2000000
if.end7 -> if.end11 : 198000000
if.then10 -> if.end11 : 2000000
if.end11 -> if.then14 : 2000000
if.end11 -> if.end15 : 198000000
if.then14 -> if.end15 : 2000000
if.end15 -> if.then18 : 2000000
if.end15 -> if.end19 : 198000000
if.then18 -> if.end19 : 2000000
if.end19 -> if.then22 : 2000000
if.end19 -> if.end23 : 198000000
if.then22 -> if.end23 : 2000000
if.end23 -> if.then26 : 2000000
if.end23 -> if.end27 : 198000000
if.then26 -> if.end27 : 2000000
if.end27 -> if.then30 : 2000000
if.end27 -> if.end31 : 198000000
if.then30 -> if.end31 : 2000000
if.end31 -> if.then34 : 2000000
if.end31 -> if.end35 : 198000000
if.then34 -> if.end35 : 2000000
if.end35 -> while.cond : 200000000
END_PROFILE
