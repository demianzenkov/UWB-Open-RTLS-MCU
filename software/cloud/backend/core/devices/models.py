from django.db import models
from django.dispatch import receiver
from services.socket.senders import send_read_network_settings_command, send_write_network_settings_command
import logging
import socket

logger = logging.getLogger('root')


class Tag(models.Model):
    notes = models.CharField(max_length=1024)


class Anchor(models.Model):
    ip = models.CharField(max_length=16)
    subnet_mask = models.CharField(max_length=16)
    server_ip_address = models.CharField(max_length=16)
    server_port = models.IntegerField()
    notes = models.CharField(max_length=1024)

    def __str__(self):
        return f'({self.id}) {self.ip}'


_WATCHING_FIELDS = ['ip', 'subnet_mask', 'server_ip_address', 'server_port']


@receiver(models.signals.pre_save, sender=Anchor, dispatch_uid="send_write_command")
def send_write_command(sender, instance, **kwqargs):

    try:
        db_obj = sender.objects.get(pk=instance.pk)
    except sender.DoesNotExist:
        pass
    else:
        should_send_command = False
        for field in _WATCHING_FIELDS:
            if not getattr(db_obj, field) == getattr(instance, field):
                should_send_command = True
                break
        if should_send_command:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.connect((instance.ip, 5005))
            logger.info(f'Sending write command to {instance.ip}')
            send_write_network_settings_command(
                sock, db_obj.ip, instance.ip, instance.subnet_mask, instance.server_ip_address, instance.server_port)
