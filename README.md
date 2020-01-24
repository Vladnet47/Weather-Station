# Weather Station

## Table of Contents
- [Description](#Description)
- [Downloading the Repository](#Downloading-the-Repository)
- [Using the Service](#Using-the-Service)
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

<br/>

## Downloading the Repository
Navigate to a directory of your choice and execute the following command:
```bash 
git clone https://github.com/Vladnet47/Weather-Station.git
```

<br/>

## Using the Service
Once everything is set up, you can modify the [database][postgres_overview], send HTTP requests to the [API][flask] and visualize the data with [Grafana][grafana_overview]. For information about setting up the weather station and server, see [Setting up Weather Station](#Setting-up-Weather-Station) and [Setting up Server](#Setting-up-Server).

#### Database
To access and modify the database, use [pgAdmin][pgadmin_install] (more on this in [Configuring PostreSQL][pgadmin_tutorial]). As long as you know the IP Address of the server, this can be done from any computer.
- Default port: 5432
- Default username: postgres
- Default password: postgres

#### API Requests
Once the server is running, it exposes three API endpoints on port 8080.
```
POST server_ip:8080/weatherstation/windrain
```
The weather station microcontroller calls this endpoint. POST request body must be ***winddir=#&windspeed=#&rainfall=#***.
```
GET server_ip:8080/ping
```
A successful response status means the API is running.
```
GET server_ip:8080/help
```
A successful response status means the API can connect to the database.

<br/>

#### Grafana
To view the data visualization, open your browser and type in `server_ip:3000`, which connects to the Grafana process running on the server.
- Default port: 3000
- Default username: admin
- Default password: grafana

<br/>

## Setting up Weather Station
### Prerequisites
Ensure you have access to the following:
- [Sparkfun ESP32 Thing][esp32_purchase]
- [Weather Meter][wm_purchase]
- [Weather Sensor Shield][ws_purchase]
- [Arduino IDE][arduino_ide]
- Internet connection
- Power source

<br/>

### Assembling Equipment
Follow these steps to set up the weather station hardware. Make sure the ESP32 is encased in a weather-resistant container.
1. [Assemble the weather meter][wm_assemble]
2. [Assemble the weather sensor shield][ws_assemble]
3. Attach the sensors to the respective ports on the shield
4. Connect the ESP32/shield to a power source

<br/>

### Uploading Program
Follow these steps to configure and upload code to the ESP32:
1. Navigate to ***Weather-Station/Microcontroller/***
2. Open ***Config.h*** in a text editor
3. Input your network details, as well as IP and port of server
```cpp
// MUST BE MODIFIED
const char NETWORK[] = "network_name";						
const char PASSWORD[] = "network_password";
const IPAddress SERVER(27, 0, 0, 1);
// END
```
4. Calculate wind direction offset expirementally using the sensor, so that 0 degrees points North
```cpp
const float WIND_DIRECTION_OFFSET = 0.0;
```
5. You don't have to change the rest, but you can modify the retry-connection delay in case the network goes down, the frequency of POST requests, and the server API endpoint
```cpp
// Network
const unsigned long WIFI_CONNECT_RETRY_DELAY = 1000;
const int PORT = 8080;

// POST request frequency (ms)
const unsigned long UPDATE_INTERVAL = 10000;

// POST request to server API
const String API_ENDPOINT = "/weatherstation/windrain";
const String VARNAME_WIND_DIRECTION = "winddir";
const String VARNAME_WIND_SPEED = "windspeed";
const String VARNAME_RAINFALL = "rainfall";
```
6. Save and exit
7. Open ***Controller.ino*** with Arduino IDE
8. Install ESP32 core for Arduino [through the Arduino IDE Boards Manager][esp32_arduino]
9. Connect the ESP32 to the computer
10. Under ***Tools***, select the desired ***Board***, ***Upload Speed*** (115200 recommended), and ***Port***
11. Upload to board

The ESP32 immediately tries to connect to the specified wireless network. Upon success, the onboard LED blinks three times. Should the network go down, the ESP32 automatically tries to reconnect.

After successfully connecting, the ESP32 continuously reads from the sensors and sends data to the server every 10 seconds. Upon a successful request, the onboard LED blinks two times. 

<br/>

## Setting up Server
### Installing Docker
The server is broken down into modules that use Docker. Follow the installation guides below.
1. [Docker][docker_install]
2. [Docker-compose][docker_c_install] (if using Raspberry PI as server, [this is a helpful link][docker_rasp])

Common commands are listed below. For additional reading, see [Docker documentation][docker_doc] and [docker-compose documentation][docker_c_doc].

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
| `docker network ls` | lists all Docker networks |
| `docker network inspect network_name` | shows containers attached to the network |

<br/>

### Mounting Volumes
This allows the database and grafana containers to [store data on the host][docker_mounting], preventing data loss upon container removal. The directory paths can be configured in the *docker-compose.yml* files for database and grafana containers.
```bash
sudo cd /var/storage
sudo mkdir postgres grafana
sudo chmod uga+rwx postgres grafana
```

<br/>
    
### Configuring PostgreSQL
Use pgAdmin, a tool specifically designed for PostgreSQL.
1. Copy ***Weather-Station/Server*** directory to your server
3. Start the database container
```bash
cd Server/database
docker-compose up -d
```
4. Install [pgAdmin][postgres_overview]
5. Connect to your database via port 5432 using [this tutorial][pgadmin_tutorial]
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
Once the database is set up, you can attach a data visualization.
1. If not done so already, copy ***Weather-Station/Server*** directory to your server
2. Start the grafana container
```bash
cd Server/grafana
docker-compose up -d
```
3. Connect to Grafana via port 3000 and use [this tutorial][grafana_tutorial] to create some visualizations.
    *Default username: admin*
    *Default password: admin*

<br/>

### Launching Server
Once PostgreSQL and Grafana are set up, the entire server is ready to be launched.
1. Build docker images for API and database manager.
3. **(Option 1)** Launch the entire system
```bash
cd Server
chmod +x build_all.sh
./build_all.sh
docker-compose up -d
```
4. **(Option 2)** Launch each container individually, but **launch database first**.
```bash
// Only for API and database manager
cd Server/container_name
chmod +x build.sh
./build.sh

// For all containers
cd Server/container_name
docker-compose up -d
```

<br/>

### Configuring Docker Containers
Docker containers are configured through the *docker-compose.yml* files, which allow the user to pass environment variables to the container. This section describes what environment variables are passed.

The server is comprised of four Docker containers:
- Database, using [PostgreSQL][postgres_overview]
- [Grafana][grafana_overview], a visualization tool
- API, which parses data from incoming requests
- Database manager, which erases outdated data (>14 days out)

<br/>

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
| API_HOST | 0.0.0.0 | specifies IP addresses that the API is listening for |
| API_PORT | 8080 | specifies port of API |
| port | 8080:8080 | maps 8080 port on server to 8080 port on API container |
| DATABASE_HOST | database | database container name |
| DATABASE_NAME | WeatherStation | database table name |
| DATABASE_USER | postgres | database username |
| DATABASE_PASSWORD | postgres | database password |
| DATABASE_PORT | 5432 | database container port |
| API_ENDPOINT | /weatherstation/windrain | API endpoint for sending data to |
| PARAM_WIND_DIRECTION | winddir | parameter name for wind direction in POST request |
| PARAM_WIND_SPEED | windspeed | parameter name for wind speed in POST request |
| PARAM_RAINFALL | rainfall | parameter name for rainfall in POST request |

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

<br/>

## Helpful Links
#### Weather Station
[Sparkfun ESP32 Thing][esp32_purchase]

[Purchasing Weather Meter][wm_purchase]

[Assembling Weather Meter][wm_assemble]

[Purchasing Weather Sensor Shield][ws_purchase]

[Assembling Weather Sensor Shield][ws_assemble]

[Downloading Arduino IDE][arduino_ide]

[Installing ESP32 Support for Arduino][esp32_arduino]

#### Docker
[Docker Overview][docker_doc]

[Installing Docker][docker_install]

[Docker-Compose Overview][docker_c_doc]

[Installing Docker-Compose][docker_c_install]

[Installing on Raspberry PI][docker_rasp]

#### PostgreSQL
[Overview][postgres_overview]

[Installing with Docker][postgres_install]

[Installing PgAdmin][pgadmin_install]

[Creating Database][pgadmin_tutorial]

#### Grafana
[Overview][grafana_overview]

[Installing with Docker][grafana_install]

[Creating Visualization][grafana_tutorial]

[arduino_ide]: https://www.arduino.cc/en/main/software
[esp32_purchase]: https://www.sparkfun.com/products/13907
[esp32_arduino]: https://learn.sparkfun.com/tutorials/esp32-thing-hookup-guide#installing-via-arduino-ide-boards-manager
[wm_purchase]: https://www.sparkfun.com/products/8942
[wm_assemble]: https://learn.sparkfun.com/tutorials/weather-meter-hookup-guide
[ws_purchase]: https://www.sparkfun.com/products/13956
[ws_assemble]: https://learn.sparkfun.com/tutorials/esp32-environment-sensor-shield-hookup-guide
[flask]: https://flask-restful.readthedocs.io/en/latest/
[flask_deploy]: https://flask.palletsprojects.com/en/1.1.x/deploying/
[docker_doc]: https://docs.docker.com/
[docker_install]: https://docs.docker.com/install/
[docker_c_doc]: https://docs.docker.com/compose/
[docker_c_install]: https://docs.docker.com/compose/install/
[docker_rasp]: https://dev.to/rohansawant/installing-docker-and-docker-compose-on-the-raspberry-pi-in-5-simple-steps-3mgl
[docker_mounting]: https://docs.docker.com/storage/volumes/
[postgres_overview]: https://www.postgresql.org/
[postgres_install]: https://hub.docker.com/_/postgres
[pgadmin_install]: https://www.pgadmin.org/download/
[pgadmin_tutorial]: https://www.pgadmin.org/docs/pgadmin4/development/getting_started.html
[grafana_overview]: https://grafana.com/grafana/
[grafana_install]: https://grafana.com/docs/grafana/latest/installation/docker/
[grafana_tutorial]: https://grafana.com/docs/grafana/latest/guides/getting_started/


