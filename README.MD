# Convert shattered assembly
Although shattered assembly can be created from some native CAD formats whose sub-components reside in individual files such as CATIA V5, Creo - Pro/E, NX or SolidWorks, it is not possible to create from a monolithic assembly file such as IFC, Revit or 3DXML.<br> 
This sample creates shattered assembly from a monolithic assembly file using HOOPS Exchange and LibConverter of HOOPS Communicator. Created shattered parts and master XML file can be loaded using HOOPS Communicator WebViewer APIs: `loadSubtreeFromXmlFile` and `loadSubtreeFromScsXmlFile`<br>
This sample was developed for the following article of Tech Soft 3D Forum:<br>
https://forum.techsoft3d.com/t/how-to-create-shattered-assembly-parts-xml-from-monolithic-assembly/2719

## Dependencies
### SDKs in use (version)
* HOOPS Communicator (2024.1.0)
* HOOPS Exchange (2024.1.0)

### Tested platforms
* Ubuntu server 20.04 LTS AWE EC2 instance
* Windows 10
* MacOS 12.4

### Build convertScShattered
convertScShattered is a server side console application which is using HOOPS Exchange and HC libconverter. 
1. Unzip HOOPS Exchange SDK and locate a valid hoops_license.h in `includ/` 
2. Unzip HOOPS Communicator SDK and locate a valid hoops_license.h in `authoring/libconverter/include/`

Linux / MacOS
3. Edit convertScShattered/Makefile according to your environment
4. Build `sudo make`

Windows
3. Edit convertScShattered¥_VS2019.bat accoording to your environment and open convertScShuttered.sln using the bat file
4. Build the solution

## Convert shattered assembly
Linux / MacOS
1. Switch to root user `sudo su -`
2. Set library path<br>
  Linux<br>
    `export LD_LIBRARY_PATH=/path/to/HOOPS_Communicator/authoring/converter/bin/linux64`<br>
  MacOS<br>
    `export DYLD_LIBRARY_PATH=/path/to/HOOPS_Communicator/authoring/converter/bin/macos`<br>
3. Run `./convertScShattered /path/to/monolithic/assembly/file /path/to/master/xml/file /path/to/SC/model/dir /path/to/SCS/model/dir /path/to/PRC/model/dir` 

Windows
1. Run `convertScShattered /path/to/monolithic/assembly/file /path/to/master/xml/file /path/to/SC/model/dir /path/to/SCS/model/dir /path/to/PRC/model/dir` 
\* `/path/to/SC/model/dir`, `/path/to/SCS/model/dir` or `/path/to/PRC/model/dir` can be "" or eliminated if you don't need to export. 

## Start demo client
1. Locate all html and js files of quick_start to <Communicator SDK>/quick_start
2. Start Communicator SC server (<Communicator SDK>/quick_start/start_server)
3. Open <br>
  SCS: `http://localhost:11180/shattered_index.html?viewer=SCS&instance=_empty`<br>
  CSR: `http://localhost:11180/shattered_index.html?viewer=CSR&instance=_empty`<br>
    
