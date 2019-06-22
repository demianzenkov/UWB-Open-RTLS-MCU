from django.contrib import admin
from .models import *


class TagAdmin(admin.ModelAdmin):
    pass


class AnchorAdmin(admin.ModelAdmin):
    pass


admin.site.register(Tag, TagAdmin)
admin.site.register(Anchor, AnchorAdmin)
