
# COMP 7500: Project 2: pWordCount: A Pipe-based WordCount Tool 
# Thanh Tin Nguyen - 904285164 
# ttn0011@auburn.edu
# Date: 2/9/2023
# Auburn University

# compile pwordcount
pwordcount: pwordcount.c utils.c
	echo "Compiling pwordcount..."
	gcc -o pwordcount pwordcount.c utils.c -std=c99
	echo "Finished Compiling."

# delete old pwordcount
clean:
	echo "Removing pwordcount executable..."
	rm -f pwordcount
	echo "Finished Cleaning." 			
