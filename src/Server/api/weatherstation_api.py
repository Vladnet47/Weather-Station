import os
from flask import Flask, request
from flask_restful import Resource, Api, reqparse
import psycopg2
from datetime import datetime

app = Flask(__name__)
api = Api(app)

# Accepts data from weather station and inserts it into database
class WeatherStation(Resource):

    # Accepts the following data:
    # Matching passphrase
    # Wind direction in degrees, clockwise from north
    # Wind speed in miles per hour
    # Rain fall in inches since last update
    def post(self):
        # Make sure that request contains all required attributes
        if not all(key in request.form for key in ('winddir', 'windspeed', 'rainfall')):
            return "Please specify values for winddir', 'windspeed', and 'rainfall' in POST body (ex. winddir=0.1&windspeed=2.1&rainfall=0.5).", 400 # bad request

        windDirection = request.form[os.getenv('PARAM_WIND_DIRECTION', 'winddir')]
        windSpeed = request.form[os.getenv('PARAM_WIND_SPEED', 'windspeed')]
        rainfall = request.form[os.getenv('PARAM_RAINFALL', 'rainfall')]

        # Create database connection
        databaseConnection = connectToDatabase()
        if databaseConnection is None:
            return 'Unable to connect to database. Make sure database container is running.', 500 # server error

        insertIntoDatabase(databaseConnection, windDirection, windSpeed, rainfall)

        return 'Successfully inserted data (winddir = ' + str(windDirection) + ', windspeed = ' + str(windSpeed) + ', rainfall = ' + str(rainfall) + ').', 200 # ok

# Test method to make sure client is able to connect to API
class Ping(Resource):
    def get(self):
        return 'API is running.', 200 # ok

# Test method to make sure api can connect to database
class Help(Resource):
    def get(self):
        databaseConnection = connectToDatabase()
        if databaseConnection is None:
            return 'Unable to connect to database. Make sure database container is running.', 500 # server error

        databaseConfigParams = databaseConnection.get_dsn_parameters()
        if databaseConnection is not None:
            databaseConnection.close()

        return 'Successfully able to connect to database with attributes: ' + str(databaseConfigParams), 200 # ok

# Appends UTC timestamp and inserts specified values into database
def insertIntoDatabase(databaseConnection, windDirection, windSpeed, rainfall):
    try:
        # Create cursor and execute data insertion
        cursor = databaseConnection.cursor()
        cursor.execute('INSERT INTO "WindRainMeasurement"("timestamp", wind_direction, wind_speed, rainfall) VALUES (%(ts)s, %(wd)s, %(ws)s, %(rf)s);', 
            { 'ts': datetime.utcnow(), 'wd': windDirection, 'ws': windSpeed, 'rf': rainfall })

        # Commit the insertion
        databaseConnection.commit()
        print("Successfully inserted data (winddir = " + str(windDirection) + ", windspeed = " + str(windSpeed) + ", rainfall = " + str(rainfall) + ")")

        cursor.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print("Failed to insert data with error:")
        print(error)
    finally:
        if databaseConnection is not None:
            databaseConnection.close()
            print("Database connection closed.")

# Connects to postgres database
def connectToDatabase():
    connection = None

    try:
        connection = psycopg2.connect(
            host=os.getenv('DATABASE_HOST', 'localhost'), 
            database=os.getenv('DATABASE_NAME'), 
            user=os.getenv('DATABASE_USER', 'postgres'), 
            password=os.getenv('DATABASE_PASSWORD', 'postgres'), 
            port=os.getenv('DATABASE_PORT', '5432')
        )

        # Print database properties
        if connection is not None:
            print("Connected to database")

    except (Exception, psycopg2.OperationalError) as error:
        print("Failed to connect to database with error:")
        print(error)
    finally:
        return connection

api.add_resource(WeatherStation, os.getenv('API_ENDPOINT', '/weatherstation/windrain'))
api.add_resource(Ping, '/ping')
api.add_resource(Help, '/help')

if __name__ == '__main__':
    app.run(host=os.getenv('API_HOST', '0.0.0.0'), port=os.getenv('API_PORT', '8080'))