from django.contrib import admin
from .models import *
from services.socket.senders import send_read_network_settings_command, send_write_network_settings_command


class TagAdmin(admin.ModelAdmin):

    pass


class AnchorAdmin(admin.ModelAdmin):

    pass


admin.site.register(Tag, TagAdmin)
admin.site.register(Anchor, AnchorAdmin)
