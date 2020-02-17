FROM python:3.7-alpine
RUN apk add postgresql-dev build-base

RUN mkdir /app
WORKDIR /app

COPY requirements.txt /app
RUN pip install -r requirements.txt

ENV PYTHONUNBUFFERED 1

COPY app.py /app

EXPOSE 5005-5100
EXPOSE 5005-5100/udp

CMD python app.py
