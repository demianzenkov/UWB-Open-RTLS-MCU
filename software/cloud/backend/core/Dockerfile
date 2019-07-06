FROM python:3.7-alpine
RUN apk update && apk add postgresql-dev gcc python3-dev musl-dev

RUN mkdir /app
WORKDIR /app
COPY requirements.txt /app

RUN pip install -r requirements.txt

COPY . /app

CMD adev runserver app.py
