=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 2000000
while.body -> if.end : 198000000
if.then -> if.end : 2000000
if.end -> if.then4 : 2000000
if.end -> if.end5 : 198000000
if.then4 -> if.end5 : 2000000
if.end5 -> if.then10 : 2000000
if.end5 -> if.end11 : 198000000
if.then10 -> if.end11 : 2000000
if.end11 -> if.then16 : 2000000
if.end11 -> if.end17 : 198000000
if.then16 -> if.end17 : 2000000
if.end17 -> if.then22 : 2000000
if.end17 -> if.end23 : 198000000
if.then22 -> if.end23 : 2000000
if.end23 -> if.then28 : 2000000
if.end23 -> if.end29 : 198000000
if.then28 -> if.end29 : 2000000
if.end29 -> if.then34 : 2000000
if.end29 -> if.end35 : 198000000
if.then34 -> if.end35 : 2000000
if.end35 -> if.then40 : 2000000
if.end35 -> if.end41 : 198000000
if.then40 -> if.end41 : 2000000
if.end41 -> if.then46 : 2000000
if.end41 -> if.end47 : 198000000
if.then46 -> if.end47 : 2000000
if.end47 -> if.then52 : 2000000
if.end47 -> if.end53 : 198000000
if.then52 -> if.end53 : 2000000
if.end53 -> while.cond : 200000000
END_PROFILE
