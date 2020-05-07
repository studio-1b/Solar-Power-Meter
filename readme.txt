UML deployment

+-----------------------+
| E-paper display       |
|   updated every 10min |
+-----------------------+
            ^
+-----------|-----------+
| Arduino   |           |
|  +------------------+---A1--> 5:1 current divider --> 12v (0 to 16.5v range) battery source
|  |VanPowerMeter.ino +---A2--> 5:1 current divider --> potentiometer --> (0 to >16.5v range) battery source 
|  |                  +---A3--> current sensor (3.3v logic, 1.676v is 0amps)  --> Battery positive lead
|  |                  +---A3--> current sensor (3.3v logic, 1.676v is 0amps)  --> Solar panel positive lead 
|  +------------------+---GND--> Ground of battery
|           |           |
|           V           |
|  +------------------+ |
|  |arduino_secrets.h | |
|  +------------------+ |
+-----------|-----------+
            |
            V
    +------Wifi Network----+
    |                      |
    V                      V
+-----+    +----------------------------------------------------------------------------+
| NTP |    | Linux Server                                                               |
+-----+    | +---------+   +-----------------------------+   +----------------------+   |
           | | Crontab |-->| pollpowerarduino.sh         |   | Createpowerdb.my.sql |   |
           | +---------+   +-----------------------------+   +----------------------+   |
           |                   |          |           |           | Install Tables      |
           |                   V          V           V           V                     |
           |               +------+   +-------+   +-----------------------------+       |
           |               | Bash |   | curl  |   | MySQL                       |       |
           |               +------+   +---|---+   +-----------------------------+       |
           +------------------------------|---------------------------------------------+
                                          |
                                          V
                                      192.168.1.200 (Presumably the Arduino above)

* Arduino does not have RTC clock with battery backup to retain time w/o power, therefore it gets it from NTP server
** it will update from NTP server regularly, and it is meant to be able to run w/o NTP for extended time as it as a drift calculator but there seems to be a bug in routine when unable to contact NTP server, the time gets corrupted.
*** It will offer SNTP responses, if it has the time.  Though this is mostly if NTP service is intermittent and other devices need SNTP server.  This use case is used very rarely
**** It will offer status of last 24hours of data, thru HTTP port 80
***** Please look at the code for A1, A2, A3, A4.  I don't think those are the defaults for the respective sensors above (the diagram is misleading)


Hardware
1. Arduino IoT Nano 33
  This is 3.3v logic Arduino, hence the 33 in name.  
  All I/O pins, analog and digital, are 3,3v logic (not 5v logic like earlier Arduinos)
  But will accept 5v power.
  This Arduino is necessary for the level of RAM required for 
    ePaper and WifiNina libraries, 
    storing 24hours of data, 
    and the buffer space required for HTML string generation, and buffering the response from HTML requests to verify server existance
    and a drift history for NTP response, to sync time.
2. 5:1 linear voltage divider for recording voltage level up to 16.5v.  Please make sure the total resistance of the divider is at least 100k-ohms, though the Arduino has a resistor as well.
3. If you wish to sense higher voltage, a 5:1 voltage divider with a potentiometer can make a variable voltage divider of 5+:1.
4. Current sensor.  There are a variety.  I used a hall-type current sensor and hacked what I assumed was the analog response.  And fed it to the Arduino




Software
Arduino IDE
Linux
  MySQL
  Curl
ChartJs javascript library (included)

Installation
1. Construct Arduino as diagrammed, build vanpower.ino, arduino_secrets and upload compiled code, using Arduino IDE
   - have both files in same folder
   - File > Open | vanpower.ino
   - Change Wifi password in arduino_secrets.h
   - NTP server is 192.168.1.201, please change it your NTP server.  public ones are on internet.
   - Compile, and if there are any problems, goto Manage Libraries and load WifiNina, AdafruitEPD and AdaFruit GFX libraries, because the code expects them to be there.  Maybe also RTCZero, SD, ArduinoBLE libraries.  This wasn't a work project, so I wasn't keeping count.
   - finally connection Arduino to PC, make sure it is recognized and press Upload
   - power up arduino and try to connect to arduino's IP address.  It should be displayed on e-paper
     but it should be static to 192.168.1.200 (change if you want, but the linux code expects it at 192.168.1.200)
     http://192.168.1.200 should display the valid urls for specific data you are looking for, but mostly that is going to be the last 24-hours of current and voltage data
2. Copy Linux code to a linux server
   - install mysql "sudo apt-get install mysql-server"
   - install curl "sudo apt-get install mysql-server"
   - install apache "sudo apt-get install apache2"
   - copy files in HTML folder to /var/www/html (this should be root of where apache serves files)
     If the folder is different, the pollpowerarduino.sh (you can do it later, when you copy it) file needs be changed to replace all instances of that folder, with the correct one
   - "sudo chmod 666 /var/www/html/*.json" because all the json files need to be able to be overwritten by cron
   - "ls -l /run/shm" and check if the permissions are 777, because the crontab scripts write temporary files to that folder
     if you do not have a "/run/shm" (shared memory), run "mount" and look for a tmpfs mount.  This is probably going to a scratch ramdisk.  replace the "/run/shm" in the shell scripts, with your temporary directory, as long as it has 777 permissions.
     777 permissions means "drwxrwxrwx" is listed for the directory when "ls -l".  It means anyone can read/write or create files on this folder.
   - copy MySQL installation, Bash scripts, and crontab jobs to it's own folder and replace in every file, all strings with [mysqlpassword], with your password
   - to create database, execute "mysql -u root -p < createpowerdb.my.sql" in mysql installation(there should be a password in this file)
   - copy crontab.sample.txt contents, and then type "sudo crontab -e" and paste the contents at end of file
     Each line of the file is supposed to request data from arduino, parse it, and insert it into MySQL
     One of the json files should be updated with every execution of the cron job.
   - to view data stored, goto [linux server IP]/vanpower.html
     It uses Chartjs on the browser to display json data in the .json files constantly updated by crontab on the server.  
     The javascript on the page will poll using XmlHttpRequest to get the json files to update the browser.  The updates are independent. 
     There is no error checking or handling.  If it fails, the data will stop updating, and you have to go investigate why.


Testing
*When it's installed correctly, the arduino will boot, show a splash, try to connect w wifi network and displays to you, then NTP server and display the time, then looks for the Linux server and shows you the BMP file you copied there.
*Then it will read the voltage and current data, and update the epaper display every 10min.
*If you visit the Arduino's webserver, with a web browser on another machine on same Wifi network and enter http://192.168.1.201/powerhistory (the address may be different if you changed it), you can see the last 24 hours of data.  If you can connect but get no response to the Arduino web server, which is about 5% of the time, a reboot will rememdy this.  The cause is unknown.

*If you're on the Linux server, you can check it's connectivity to the Arduino by issuing command "curl http://192.168.1.201/powerhistory" and you'll see the raw html of the data above.  If you don't connect, the linux server isn't on the same wifi network or a firewall is blocking.
*You can enter MySql command line with "Mysql -u root -p vanpower" and then "SELECT * FROM minute_by_minute;" and this is one table that is in vanpower database.  This table should be have record populated every 10min, if everything in MYySQL is installed correctly.
*if not, you can MySql command line with "Mysql -u root -p vanpower" and then "SELECT * FROM minute_by_minute_import;" and this is one table that is in vanpower database.  This table should be have new records populated every 10min, if crontab is configured correctly and pollpowerarduino.sh is functioning correctly.
*if not, you can run "./pollpowerarduino.sh intervalaverages" and see if the tables above are populated.
*if not, you can browse the pollpowerarduino.sh and debug it, but every 10min there should be a file named /run/shm/powerhistory.csv updated, and it contains the same HTML as running the curl command.

*Once you are confident the tables update correctly, you view if the json files (latestintervalaverages.json) in /var/www/html are being updated every 10min.
*Then you can goto a browser (chrome works well, internet explorer does not) and visit http://192.168.1.201/vanpower and the webpage will load the data from the json files every 10min.  Clock displays time of last data point.


If every works!  You can view and record the power being used by your off-grid solar controller.

ENJOY!
