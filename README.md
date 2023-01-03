# ARP-SharedMemory-Assignment
Second *Advanced and Robot Programming* (ARP) assignment. This assignment works with the ncurses library, konsole and bitmap, so it is necessary to have them installed. There are provided two folders, one called src that include the c files for each process and other called include that contain the header of each c file. Once the program will be executed there will appear two more folders: bin, that will contain the executables and out, that will contain the .bmp files each time the user will press the print button.

This assigment is composed of two main processes that, by reading and writing in a shared memory, are able to comunicate the position of a circle. Both process have a dynamic private memory and a static shared memory. Both memories keep updated all the time. Notice that the memories have a dimension 20 times bigger than the windows that the user sees.
## konsole installation
To install the ncurses library, simply open a terminal and type the following command:
```console
sudo apt-get install konsole
```

## ncurses installation
To install the ncurses library, simply open a terminal and type the following command:
```console
sudo apt-get install libncurses-dev
```

## Compiling and running the code
The assignment provides a shell script that compiles and executes the code. Please, notice that for the first time, once the repository has been clone, is necesary to give permissions:
```console
chmod +x script.sh
```
Once the permission have been done, the assignment is executed by typing:
```console
./script.sh
```
After compiling,the user will be able to see two screens, the process A and the process B windows.

## How the assignment works
The assignment is divided into 3 processes: A, B and master.
* Master process: This process is the main process. It creates the other processes A and B. Notice that process A has always to be executed first, because is the process that initialize the semaphores, and at the time that process B opens the semaphores, those have to be already initialized.
* A process: This process reads from the input of the user. It admits the 4 movements with the arrows of the keyboard, up, down, right or left. If any of this keys are pressed and after the limits are considered, the green cross will move. This green cross represents a ball. When the position changes, the private dynamic memory and the static shared memory are updated. This window also provides a print button, that lets the user generate a .bmp file (in the folder out) with an image of the position of the circle.
* B process: This process reads from the shared memory and keeps updating in the window the trace of the movement of the circle. This process is also writing in its dynamic private memory the information of the last circle position.
Notice that to avoid any problem for the critical region, semaphores are used.

Here is presented a brief scheme of how the assignment is working:
![SCHEME](https://github.com/isacg5/second_assignmentARP/blob/main/resources/image.jpg)