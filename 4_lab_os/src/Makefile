all: Program_1 Program_2 libPr_2_real1.so libPr_2_real2.so clean

Program_1: Program_1.o libPr_2_real1.so
	g++ -o Program_1 Program_1.o -L. -lPr_2_real1 -Wl,-rpath,.
Program_1.o: Program_1.cpp
	g++ -c Program_1.cpp
Pi_Leibniz.o: Pi_Leibniz.cpp
	g++ -c Pi_Leibniz.cpp
Square_rect.o: Square_rect.cpp
	g++ -c Square_rect.cpp

Program_2: Program_2.o
	g++ -o Program_2 Program_2.o -ldl
Program_2.o: Program_2.cpp
	g++ -c Program_2.cpp
libPr_2_real1.so: Pi_Leibniz.o Square_rect.o
	g++ -shared -o libPr_2_real1.so Pi_Leibniz.o Square_rect.o
libPr_2_real2.so: Pi_Wallis.o Square_tr.o
	g++ -shared -o libPr_2_real2.so Pi_Wallis.o Square_tr.o
Pi_Wallis.o:
	g++ -c Pi_Wallis.cpp
Square_tr.o:
	g++ -c Square_tr.cpp

clean:
	rm -f *.o