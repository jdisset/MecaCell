#MecaCell
MecaCell is an open-source cellular physics agent-based simulation platform. It is designed with artificial life and morphogenetic engineering in mind and tries to stay not too invasive, lightweight and easily extensible.
It comes in the form of two libraries :
- **MecaCell** is the _core_ library which contains everything you need to run simulations in console or with your own viewer. It is written in C++11 and has no dependencies other than the C++ standard library.
- **MecaCellViewer** is a _viewer_ library. It is written in C++, uses OpenGL and depends on Qt5.


![screenshots](https://github.com/jdisset/MecaCell/blob/screens/githubmecacell.jpg)


##Installation
You will need cmake 2.8+, a relatively recent C++ compiler (C++11 support is required) and Qt 5.2+
- Clone this repository
- cd in it and create a build directory
- cmake .. && make && make install

##Basic usage & ultra simple example
You need at least one **cell type**.

Your cell type must inherit from the MecaCell::ConnectableCell (it uses the curriously reccuring template for performances reason, so the inherit line is a little bit more verbose):
	```c++
		 class MyCell : public MecaCell::ConnectableCell<MyCell> {
			 ```
				 a cell is required to have at least 2 methods:
				 ```c++
				 // returns the adhesion coef (between 0 & 1) with the cell *c
				 double getAdhesionWith(const MyCell *c) { return 0.9; }
			 ```
				 ```c++
				 // update routine, called at every loop iteration
				 // returns a pointer to a new cell if division
				 // returns nullptr if not
				 MyCell* updateBehavior(double deltaTime) {
					 // access inputs here
					 return nullptr;
				 }
		 };
```


Let's now create a scenario:

```c++
class MyScenario {
	```
		A scenario needs to contain a MecaCell::BasicWorld<cell_type,integration_mode> and let the viewer acces to it:
		```c++
		using World = MecaCell::BasicWorld<MyCell, MecaCell::Euler>;
	private:
	World w;

	public:
	World &getWorld() { return w; }
	```

		It should also contains at least these 2 methods:
		```c++
		// called at initialisation
		void init(int argc, char** argv) {
			// here we just add a cell at (0,0,0);
			w.addCell(new Cell(MecaCell::Vec::zero()));
		}
	```
		```c++
		void loop(){
			// this code is called before every frame by the viewer
			// here we just call the world update method, which will handle all the physics and call
			// our cells behavior method
			w.update();
			// handle events, plug your own methods call, whatever you want goes in this method...
		}
};
```
