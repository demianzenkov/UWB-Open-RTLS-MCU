from django.db import models


class Tag(models.Model):
    notes = models.CharField(max_length=1024)


class Anchor(models.Model):
    notes = models.CharField(max_length=1024)
