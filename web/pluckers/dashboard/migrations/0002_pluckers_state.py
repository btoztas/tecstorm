# -*- coding: utf-8 -*-
# Generated by Django 1.11.10 on 2018-03-04 00:54
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('dashboard', '0001_initial'),
    ]

    operations = [
        migrations.AddField(
            model_name='pluckers',
            name='state',
            field=models.BooleanField(default=False),
        ),
    ]
