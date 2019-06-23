import logging
from django.contrib import admin
from .models import *
from services.udp_sender.senders import send_read_network_settings_command, send_write_network_settings_command

logger = logging.getLogger('root')

def read_network_settings(modeladmin, request, queryset):
    for tag in queryset:
        try:
            send_read_network_settings_command(tag.ip)
        except Exception as e:
            logger.exception(e)

def write_network_settings(modeladmin, request, queryset):
    for tag in queryset:
        try:
            send_write_network_settings_command(tag.ip)
        except Exception as e:
            logger.exception(e)

class TagAdmin(admin.ModelAdmin):
    
    actions = [read_network_settings, write_network_settings]
    

class AnchorAdmin(admin.ModelAdmin):
    pass


admin.site.register(Tag, TagAdmin)
admin.site.register(Anchor, AnchorAdmin)
