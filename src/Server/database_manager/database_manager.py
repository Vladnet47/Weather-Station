import os
import psycopg2
import schedule
from datetime import datetime, timedelta
from time import sleep

# Deletes wind/rain measurements that occured more than specified amount of days ago
def deleteFromDatabase():
    databaseConnection = connectToDatabase()

    # Determine timestamp before which all entries should be deleted
    targetTimestamp = datetime.utcnow() - timedelta(
        days = int(os.getenv('TIME_WINDOW_DAYS', 14)), 
        hours = int(os.getenv('TIME_WINDOW_HOURS', 0)), 
        minutes = int(os.getenv('TIME_WINDOW_MINUTES', 0)), 
        seconds = int(os.getenv('TIME_WINDOW_SECONDS', 0))
    )

    try:
        # Create cursor and execute data deletion
        cursor = databaseConnection.cursor()
        cursor.execute('DELETE FROM "WindRainMeasurement" WHERE "timestamp" < timestamp %(time)s', { 'time': targetTimestamp })
        databaseConnection.commit()

        print("Successfully deleted data before " + targetTimestamp.strftime("%m/%d/%Y, %H:%M") + " UTC")
        cursor.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print("Failed to delete data with error:")
        print(error)
    finally:
        # Close database connection
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

# Deletion job runs every day at specified time
schedule.every(int(os.getenv('DELETION_FREQUENCY_HOURS', 24))).hours.do(deleteFromDatabase)

# Debug
#schedule.every(20).seconds.do(deleteFromDatabase)

def main():
    while True:
        schedule.run_pending()
        sleep(10)

main()