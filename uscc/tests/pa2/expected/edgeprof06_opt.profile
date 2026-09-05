=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, if.end -> if.end5, if.then4 -> if.end5, if.end5 -> if.end11, if.then10 -> if.end11, if.end11 -> if.end17, if.then16 -> if.end17, if.end17 -> if.end23, if.then22 -> if.end23, if.end23 -> if.end29, if.then28 -> if.end29, if.end29 -> if.end35, if.then34 -> if.end35, if.end35 -> if.end41, if.then40 -> if.end41, if.end41 -> if.end47, if.then46 -> if.end47, if.end47 -> if.end53, if.then52 -> if.end53, if.end53 -> while.cond
INSTRUMENTED: while.body -> if.end, if.then -> if.end, if.end -> if.then4, if.end5 -> if.then10, if.end11 -> if.then16, if.end17 -> if.then22, if.end23 -> if.then28, if.end29 -> if.then34, if.end35 -> if.then40, if.end41 -> if.then46, if.end47 -> if.then52
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.end : 198000000
if.then -> if.end : 2000000
if.end -> if.then4 : 2000000
if.end -> if.end5 : 0
if.then4 -> if.end5 : 0
if.end5 -> if.then10 : 2000000
if.end5 -> if.end11 : 0
if.then10 -> if.end11 : 0
if.end11 -> if.then16 : 2000000
if.end11 -> if.end17 : 0
if.then16 -> if.end17 : 0
if.end17 -> if.then22 : 2000000
if.end17 -> if.end23 : 0
if.then22 -> if.end23 : 0
if.end23 -> if.then28 : 2000000
if.end23 -> if.end29 : 0
if.then28 -> if.end29 : 0
if.end29 -> if.then34 : 2000000
if.end29 -> if.end35 : 0
if.then34 -> if.end35 : 0
if.end35 -> if.then40 : 2000000
if.end35 -> if.end41 : 0
if.then40 -> if.end41 : 0
if.end41 -> if.then46 : 2000000
if.end41 -> if.end47 : 0
if.then46 -> if.end47 : 0
if.end47 -> if.then52 : 2000000
if.end47 -> if.end53 : 0
if.then52 -> if.end53 : 0
if.end53 -> while.cond : 0
END_PROFILE
