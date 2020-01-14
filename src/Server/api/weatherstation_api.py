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
        if not all(key in request.form for key in ('passphrase', 'winddir', 'windspeed', 'rainfall')):
            return "Please specify values for 'passphrase', 'winddir', 'windspeed', and 'rainfall'.", 400 # bad request
        
        passphrase = request.form['passphrase']

        # Check to make sure the passphrase matches, and return error if not
        if (passphrase != os.getenv('WEATHERSTATION_PASSPHRASE')):
            return "Incorrect Passphrase!", 401 # unauthorized

        windDirection = request.form['winddir']
        windSpeed = request.form['windspeed']
        rainfall = request.form['rainfall']

        insertIntoDatabase(windDirection, windSpeed, rainfall)

        return 200 # ok

# Test method to make sure client is able to connect to API
class Ping(Resource):
    def get(self):
        return 200 # ok

# Test method to make sure api can connect to database
class Help(Resource):
    def get(self):
        databaseConnection = connectToDatabase()

        databaseConfigParams = databaseConnection.get_dsn_parameters()
        if databaseConnection is not None:
            databaseConnection.close()

        return databaseConfigParams, 200 # ok

# Appends UTC timestamp and inserts specified values into database
def insertIntoDatabase(windDirection, windSpeed, rainfall):
    databaseConnection = connectToDatabase()

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

    return connection

api.add_resource(WeatherStation, '/weatherstation/windrain')
api.add_resource(Ping, '/ping')
api.add_resource(Help, '/help')

if __name__ == '__main__':
    app.run(host=os.getenv('HOST', '0.0.0.0'), port=os.getenv('PORT', '80'))