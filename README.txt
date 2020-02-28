Eva Bruklich & Noah Parker
ebruklich nparker

We completed the project to the best of our ability and have verified that all of our tests and different combinations of inputs provided the correct outputs. We provided a sample TestInput.txt file with a collection of inputs that could be provided and then the respective outputs in the Output.txt file. 

Part 1: Memory Manager
We created a memory manager that compeletes all of the tasks indicated in the instructions for part one. We began by not worrying about swapping and then completed that functionality in Part 2, as indicated in the instructions. We followed the simulation parameters and worked around them to provide out simulation. 

Part 2: Memory Manager 2
For the eviction strategy we decided on using Least-Recently-Used (LRU) as the programs way of deciding which pages should be swapped. We created a variable, useCounter, to keep track of the least recntly used page. When a page was created it was assigned 1 and the following page was was created was assigned 2, and so on. When looking for a page to swap which was least recently used, it just had to look for the page with the smallest number assigned to it. 
