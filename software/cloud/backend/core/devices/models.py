from django.db import models


class Tag(models.Model):
    ip = models.CharField(max_length=16)
    subnet_mask = models.CharField(max_length=16)
    server_ip_address = models.CharField(max_length=16)
    server_port = models.IntegerField()
    notes = models.CharField(max_length=1024)

    def __str__(self):
        return f'({self.id}) {self.ip}'


class Anchor(models.Model):
    notes = models.CharField(max_length=1024)
