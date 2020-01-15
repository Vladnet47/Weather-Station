# Weather Station

## Table of Contents
- [Description](#Description)
- [Downloading the Repository](#Downloading-the-Repository)
- [Setting up Weather Station](#Setting-up-Weather-Station)
    - [Prerequisites](#Prerequisites)
    - [Assembling Equipment](#Assembling-Equipment)
    - [Uploading Program](#Uploading-Program)
- [Setting up Server](#Setting-up-Server)
    - [Installing Docker](#Installing-Docker)
    - [Mounting Volumes](#Mounting-Volumes)
    - [Configuring PostreSQL](#Configuring-PostgreSQL)
    - [Configuring Grafana](#Configuring-Grafana)
    - [Launching Server](#Launching-Server)
    - [Configuring Docker Containers](#Configuring-Docker-Containers)
- [FAQ](#FAQ)
- [Helpful Links](#Helpful-Links)

<br/><br/>

## Description
As a way to attract embedded systems enthusiasts, a couple of students and I are partnering with the UW Bothell Makerspace to install a weather station in the campus garden.

The weather station runs on an ESP32 microcontroller, which reads from attached weather sensors, connects to a wireless network, and sends the data over HTTP to a server running on a Raspberry PI. 

The server is comprised of four Docker containers:
- API, which parses data from incoming requests
- [PostgreSQL](https://www.postgresql.org/), a database
- Database management tool, which erases outdated data (>14 days out)
- [Grafana](https://grafana.com/grafana/), which allows external users to connect and see the data on a timescale graph

The following sections describe how to set up and configure the system.

<br/><br/>

## Downloading the Repository
Navigate to a directory of your choice and execute the following command:
```bash 
git clone https://github.com/Vladnet47/Weather-Station.git
```

<br/><br/>

## Setting up Weather Station
### Prerequisites
Ensure you have access to the following:
- [Sparkfun ESP32 Thing](https://www.sparkfun.com/products/13907)
- [Weather Meter](https://www.sparkfun.com/products/8942)
- [Weather Sensor Shield](https://www.sparkfun.com/products/13956)
- Internet connection
- Power source
- [Arduino IDE](https://www.arduino.cc/en/main/software)

<br/>

### Assembling Equipment
Follow these steps to set up the weather station hardware. Make sure the ESP32 is encased in a weather-resistant container.
1. [Assemble the weather meter](https://learn.sparkfun.com/tutorials/weather-meter-hookup-guide)
2. [Assemble the weather sensor shield](https://learn.sparkfun.com/tutorials/esp32-environment-sensor-shield-hookup-guide)
3. Attach the sensors to the respective ports on the shield
4. Connect the ESP32/shield to a power source

<br/>

### Uploading Program
Follow these steps to configure and upload code to the ESP32:
1. Navigate to ***Weather-Station/Microcontroller/***
2. Open ***Config.h*** in a text editor
3. Input your network details, as well as IP and port of server
```c++
const char NETWORK[] = "network_name";
const char PASSWORD[] = "network_password";
const IPAddress SERVER(27, 0, 0, 1);
const int PORT = 1234;
```
4. Save and exit
5. Open ***Controller.ino*** with Arduino IDE
6. Connect the ESP32 to the computer
7. Under ***Tools***, select the desired ***Board***, ***Upload Speed*** (115200 recommended), and ***Port***
8. Upload to board

The ESP32 immediately tries to connect to the specified wireless network. Upon success, the onboard LED blinks three times. Should the network go down, the ESP32 automatically tries to reconnect.

After successfully connecting, the ESP32 continuously reads from the sensors and sends data to the server every 10 seconds. Upon a successful request, the onboard LED blinks two times. 

To change the frequency of requests, open ***Weather-Station/Microcontroller/Controller.ino*** and modify the update interval (in milliseconds).
```cpp  
unsigned long UPDATE_INTERVAL = 10000;
```

You can also change the server API endpoint and parameter names.
```cpp
const String API_ENDPOINT = "/weatherstation/windrain";
const String VARNAME_WIND_DIRECTION = "winddir";
const String VARNAME_WIND_SPEED = "windspeed";
const String VARNAME_RAINFALL = "rainfall";
```

<br/><br/>

## Setting up Server
### Installing Docker
Follow the installation guides below.
1. [Docker](https://docs.docker.com/install/)
2. [Docker-compose](https://docs.docker.com/compose/install/) (if using Raspberry PI as server, [this is a helpful link](https://dev.to/rohansawant/installing-docker-and-docker-compose-on-the-raspberry-pi-in-5-simple-steps-3mgl))

Common commands are listed below. For additional reading, see [Docker documentation](https://docs.docker.com/) and [docker-compose documentation](https://docs.docker.com/compose/).

| Command | Description |
| ------- | ----------- |
| `docker ps -a` | lists all containers |
| `docker images` | lists all images |
| `docker start container_name` | starts container |
| `docker stop container_name` | stops container |
| `docker rm -v container_name` | removes container |
| `docker attach container_name` | displays container console output |
| `docker inspect container_name` | displays environment and configuration variables for container |
| `docker rmi image_name` | removes image |
| `docker-compose up -d` | launches containers in docker-compose.yml file (in same folder) |
| `docker-compose up` | stops and removes containers in docker-compose.yml file (in same folder) |

<br/>

### Mounting Volumes
This allows the database and grafana containers to store data on the host, preventing data loss upon container removal. The actual folder locations can be configured in the respective *docker-compose.yml* files.
```bash
sudo cd /var/storage
sudo mkdir postgres grafana
sudo chmod uga+rwx postgres grafana
```

<br/>
    
### Configuring PostgreSQL
Use pgAdmin, a tool specifically designed for PostgreSQL.
1. Copy ***Weather-Station/Server*** directory to your server
2. Navigate to ***Server/database***
3. Start the database container
```bash
docker-compose up -d
```
4. Install [pgAdmin](https://www.pgadmin.org/download/)
5. Connect to your database via port 5432 using [this tutorial](https://www.pgadmin.org/docs/pgadmin4/development/getting_started.html)
    *Default username: postgres*
    *Default password: postgres*
6. Add database called ***WeatherStation***
7. Under ***WeatherStation***, create table called ***WindRainMeasurements*** with the following columns:
    
| Name | Data Type | Length | Precision | Constraints |
| ---- | --------- | ------ | --------- | ----------- |
| timestamp | timestamp without time zone | 6 | | Not Null |
| wind_direction | numeric | 5 | 2 | Not Null |
| wind_speed | numeric | 5 | 2 | Not Null |
| rainfall | numeric | 5 | 2 | Not Null |

<br/>
     
### Configuring Grafana
Now that the database has been configured, you can attach a data visualization.
1. If not done so already, copy ***Weather-Station/Server*** directory to your server
1. Navigate to ***Server/grafana***
2. Start the grafana container
```bash
docker-compose up -d
```
3. Connect to grafana via port 3000 and follow [this tutorial](https://grafana.com/docs/grafana/latest/guides/getting_started/)
    *Default username: admin*
    *Default password: grafana*

<br/>

### Launching Server
Once PostgreSQL and Grafana are set up, the entire server is ready to be launched.
1. Build Docker image for API
```bash
cd Server/api
sudo chmod +x build.sh
./build.sh
```
2. Build Docker image for database management tool
```bash
cd Server/database_manager
sudo chmod +x build.sh
./build.sh
```
3. **(Option 1)** Launch the entire system
```bash
cd Server
docker-compose up -d
```
4. **(Option 2)** Launch each container individually, but **launch database first**
```bash
cd Server/container_name
docker-compose up -d
```

<br/>

### Configuring Docker Containers
Docker containers are configured through the *docker-compose.yml* files, which allow the user to pass environment variables to the container. This section describes what environment variables are passed.

#### Database
| Variable | Default Value | Description |
| ---- | --------- | ------ |
| port | 5432:5432 | maps 5432 port on server to 5432 port on database container |

<br/>

#### Grafana
| Variable | Default Value | Description |
| ---- | --------- | ------ |
| port | 3000:3000 | maps 3000 port on server to 3000 port on grafana container |

<br/>

#### API
If you change the password or username on the database, make sure to update the variables.

| Variable | Default Value | Description |
| ---- | --------- | ------ |
| port | 8080:80 | maps 8080 port on server to 80 port on api container |
| DATABASE_HOST | database | database container name |
| DATABASE_NAME | WeatherStation | database table name |
| DATABASE_USER | postgres | database username |
| DATABASE_PASSWORD | postgres | database password |
| DATABASE_PORT | 5432 | database container port |

<br/>

#### Database Manager
If you change the password or username on the database, make sure to update the variables.

| Variable | Default Value | Description |
| ---- | --------- | ------ |
| DATABASE_HOST | database | database container name |
| DATABASE_NAME | WeatherStation | database table name |
| DATABASE_USER | postgres | database username |
| DATABASE_PASSWORD | postgres | database password |
| DATABASE_PORT | 5432 | database container port |
| DELETION_FREQUENCY_HOURS | 1 | how frequently deletion job runs |
| TIME_WINDOW_DAYS | 7 | data from more than 7 days ago is deleted |
| TIME_WINDOW_HOURS | 0 | same as TIME_WINDOW_DAYS but hours |
| TIME_WINDOW_MINUTES | 0 | same as TIME_WINDOW_DAYS but minutes |
| TIME_WINDOW_SECONDS | 0 | same as TIME_WINDOW_DAYS but seconds |

<br/><br/>

## FAQ
#### Can't select the "Sparkfun ESP32 Thing" board in Arduino IDE
You must install the ESP32 core for Arduino [through the Arduino IDE Boards Manager](https://learn.sparkfun.com/tutorials/esp32-thing-hookup-guide#installing-via-arduino-ide-boards-manager)

#### Gibberish being printed in Arduino IDE Serial Monitor
Ensure that your Serial Monitor baud rate is the same as the one set under ***Tools/Upload Speed***

#### Can't connect to database in Grafana
1. Make sure database container is running
2. Make sure IP Address and port is specified correctly
3. Make sure database username and password is entered correctly
4. Make sure database name is ***WeatherStation***
5. Make sure 'verify-SSL' is set to *never*

#### Docker-compose 'container is already running'
If a container was previously launched with a different `docker-compose up` command, launching the same container with the new one throws an error.
- Navigate to directory with initial ***docker-compose.yml*** file and execute `docker-compose down`
- **(or)** Manually stop the container
```bash
docker stop container_name
docker rm container_name
```

<br/><br/>

## Helpful Links
#### Weather Station
[Sparkfun ESP32 Thing](https://www.sparkfun.com/products/13907)

[Purchasing Weather Meter](https://www.sparkfun.com/products/8942)

[Assembling Weather Meter](https://learn.sparkfun.com/tutorials/weather-meter-hookup-guide)

[Purchasing Weather Sensor Shield](https://www.sparkfun.com/products/13956)

[Assembling Weather Sensor Shield](https://learn.sparkfun.com/tutorials/esp32-environment-sensor-shield-hookup-guide)

[Downloading Arduino IDE](https://www.arduino.cc/en/main/software)

[Installing ESP32 Support for Arduino](https://learn.sparkfun.com/tutorials/esp32-thing-hookup-guide#installing-via-arduino-ide-boards-manager)

#### Docker
[Docker Overview](https://docs.docker.com/)

[Installing Docker](https://docs.docker.com/install/)

[Docker-Compose Overview](https://docs.docker.com/compose/)

[Installing Docker-Compose](https://docs.docker.com/compose/install/)

[Installing on Raspberry PI](https://dev.to/rohansawant/installing-docker-and-docker-compose-on-the-raspberry-pi-in-5-simple-steps-3mgl)

#### PostgreSQL
[Overview](https://www.postgresql.org/)

[Installing with Docker](https://hub.docker.com/_/postgres)

[Installing PgAdmin](https://www.pgadmin.org/download/)

[Creating Database](https://www.pgadmin.org/docs/pgadmin4/development/getting_started.html)

#### Grafana
[Overview](https://grafana.com/grafana/)

[Installing with Docker](https://grafana.com/docs/grafana/latest/installation/docker/)

[Creating Visualization](https://grafana.com/docs/grafana/latest/guides/getting_started/)

