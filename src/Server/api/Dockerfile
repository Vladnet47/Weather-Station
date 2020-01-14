FROM python:3

ADD weatherstation_api.py /api/

RUN pip install flask
RUN pip install flask_restful
RUN pip install psycopg2-binary

WORKDIR /api

CMD ["python3", "-u", "./weatherstation_api.py"]


