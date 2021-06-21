all:
	g++ NaSchModel-multi-demo.cpp -lncurses -O3 -o two_lane_demo
	g++ NaSchModel-multi-data.cpp -O3 -o two_lane_data 
clean:
	rm -f two_lane_demo two_lane_data 
sym:
	./two_lane_demo 20 5 5 0.3 10 1 1
asym:
	./two_lane_demo 20 5 5 0.3 10 1 0
