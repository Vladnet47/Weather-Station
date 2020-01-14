FROM python:3

ADD database_manager.py /manager/

RUN pip install schedule
RUN pip install psycopg2-binary

WORKDIR /manager

CMD ["python3", "-u", "./database_manager.py"]


