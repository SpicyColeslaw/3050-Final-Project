# 3050-Final-Project
This was the final project for my CS3050 class. A robot traverses a map to the finish while dodging objects. 

Sample.txt contains the instructions for the format of each txt file! The "sample.txt" will not work given the comments on each line. All
the other files should work though. As this was a final project, the code could have been a little cleaner but other projects caused me to 
just make it work. I would like to go back through at some point and clean it up a bit. 

File format:
12		  //room length
(1,3)	  //robot starting position
(10,9)	//robot finish position
(10,8)	//obstacle 1 starting position
1		    //obstacle 1 speed
(0,+1)	//obstacle 1 direction
(2,2)	  //obstacle 2 starting position
1		    //obstacle 2 speed
(-1,+1)	//obstacle 2 direction

Example command: "./a.out sample6.txt"
