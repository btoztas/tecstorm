# -*- coding: utf-8 -*-
from __future__ import unicode_literals
from django.contrib.auth.models import User
from django.db import models
from django.utils import timezone

# Create your models here.

class UserTag(models.Model):
    user = models.ForeignKey(User, on_delete=models.CASCADE)
    tag = models.CharField(max_length=50, null=True, blank=True)
    name = models.CharField(max_length=50, null=True, blank=True)


class Pluckers(models.Model):
    uuid = models.CharField(max_length=10, primary_key=True)
    user = models.ForeignKey(User)
    name = models.CharField(max_length=50)
    location = models.CharField(max_length=100)
    created_at = models.DateTimeField(editable=False)
    modified_at = models.DateTimeField()

    def save(self, *args, **kwargs):
        if not self.modified_at:
            self.created_at = timezone.now()
        self.modified_at = timezone.now()
        return super(Pluckers, self).save(*args, **kwargs)


class Session(models.Model):
    pluckers = models.ForeignKey(Pluckers, on_delete=models.CASCADE, related_name='pluckers_used')
    user = models.ForeignKey(User, on_delete=models.CASCADE, related_name='used_by')
    active = models.BooleanField()
    power = models.IntegerField(null=True, blank=True)
    price = models.IntegerField(null=True, blank=True)
    started_at = models.DateTimeField(editable=False)
    ended_at = models.DateTimeField(null=True, blank=True)

    def save(self, *args, **kwargs):
        if not self.started_at:
            self.started_at = timezone.now()
        else:
            self.ended_at = timezone.now()

        return super(Session, self).save(*args, **kwargs)


class Consume(models.Model):
    session = models.ForeignKey(Session, on_delete=models.CASCADE, related_name='session')
    value = models.IntegerField()
    created_at = models.DateTimeField(editable=False)

    def save(self, *args, **kwargs):
        if not self.created_at:
            self.created_at = timezone.now()
        return super(Consume, self).save(*args, **kwargs)

