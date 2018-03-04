# -*- coding: utf-8 -*-
import json

from django.contrib.auth.decorators import login_required
from django.http import HttpResponse, HttpResponseRedirect
from django.shortcuts import render
from django.utils.decorators import method_decorator
from django.views import View

from dashboard.forms import NewPluckersForm, NewTagForm
from dashboard.models import Pluckers, Consume, Session, UserTag


def get_context(request):
    context = {
        'username': request.user.username,
        'is_admin': request.user.is_staff
    }
    return context


@method_decorator(login_required(login_url='/login/'), name='dispatch')
class HomeView(View):
    template_name = 'home.html'

    def get(self, request, *args, **kwargs):
        context = get_context(request)
        return render(request, self.template_name, context)


@method_decorator(login_required(login_url='/login/'), name='dispatch')
class DevicesView(View):
    template_name = 'devices.html'

    def get(self, request, *args, **kwargs):
        context = get_context(request)
        context['list'] = Pluckers.objects.filter(user=request.user).all()

        return render(request, self.template_name, context)


@method_decorator(login_required(login_url='/login/'), name='dispatch')
class NewDeviceView(View):
    template_name = 'new_device.html'

    def get(self, request, *args, **kwargs):
        context = get_context(request)

        return render(request, self.template_name, context)

    def post(self, request):
        # create a form instance and populate it with data from the request:
        form = NewPluckersForm(request.POST)
        # check whether it's valid:

        if form.is_valid():
            user = request.user
            uuid = form.cleaned_data['uuid']
            name = form.cleaned_data['name']
            location = form.cleaned_data['location']

            new_pluckers = Pluckers(user=user, uuid=uuid, location=location, name=name)
            new_pluckers.save()

            return HttpResponseRedirect('/devices/' + uuid)


@method_decorator(login_required(login_url='/login/'), name='dispatch')
class DeviceView(View):
    template_name = 'device.html'

    def get(self, request, *args, **kwargs):
        context = get_context(request)

        uuid = kwargs['uuid']

        context['pluckers'] = Pluckers.objects.filter(uuid=uuid).first()
        context['active_session'] = Session.objects.filter(pluckers=context['pluckers'], active=True).first()
        context['sessions'] = Session.objects.filter(pluckers=context['pluckers'], active=False).all()

        return render(request, self.template_name, context)


@method_decorator(login_required(login_url='/login/'), name='dispatch')
class SessionsView(View):
    template_name = 'sessions.html'

    def get(self, request, *args, **kwargs):
        context = get_context(request)

        context['active_sessions'] = Session.objects.filter(user=request.user, active=True).all()
        context['sessions'] = Session.objects.filter(user=request.user, active=False).all()

        return render(request, self.template_name, context)


@method_decorator(login_required(login_url='/login/'), name='dispatch')
class TagsView(View):
    template_name = 'tags.html'

    def post(self, request, *args, **kwargs):

        form = NewTagForm(request.POST)

        if form.is_valid():
            uuid = form.cleaned_data['uuid']
            name = form.cleaned_data['name']

            new_tag = UserTag(tag=uuid, user=request.user, name=name)
            new_tag.save()

            return HttpResponseRedirect('/tags')

    def get(self, request, *args, **kwargs):
        context = get_context(request)

        context['tags'] = UserTag.objects.filter(user=request.user).all()

        return render(request, self.template_name, context)


class SessionApiView(View):
    def post(self, request, *args, **kwargs):

        body = json.loads(request.body)

        if 'tag' in body and 'pluckers' in kwargs:

            tag = body['tag']
            pluckers_uuid = kwargs['pluckers']

            try:
                user_tag = UserTag.objects.get(tag=tag)
                user = user_tag.user
                pluckers = Pluckers.objects.filter(uuid=pluckers_uuid).first()

            except (UserTag.DoesNotExist, ValueError):

                response = dict()
                response['error'] = 'tag not found'
                return HttpResponse(
                    json.dumps(response),
                    content_type='application/json',
                    status=404
                )

            # Check if there is an active session

            active_session = Session.objects.filter(active=True, user=user, user_tag=user_tag, pluckers=pluckers).first()

            if active_session:

                # End session
                active_session.active = False

                # Calculate the power and price
                active_session.power = 10
                active_session.price = 10
                active_session.save()
                pluckers.state = False
                pluckers.save()

            else:

                new_session = Session(user=user, user_tag=user_tag, pluckers=pluckers, active=True)
                new_session.save()
                pluckers.state = True
                pluckers.save()

            return HttpResponse(
                status=200,
                content_type='application/json',
            )

        response = dict()
        response['error'] = 'bad request'
        return HttpResponse(
            json.dumps(response),
            content_type='application/json',
            status=400
        )

class ConsumeApiView(View):

    def post(self, request, *args, **kwargs):

        body = json.loads(request.body)

        if 'value' in body and 'pluckers' in kwargs:

            value = body['value']
            pluckers_uuid = kwargs['pluckers']

            try:
                pluckers = Pluckers.objects.filter(uuid=pluckers_uuid).first()
                session = Session.objects.filter(active=True, pluckers=pluckers).first()

            except (UserTag.DoesNotExist, ValueError):

                response = dict()
                response['error'] = 'tag not found'
                return HttpResponse(
                    json.dumps(response),
                    content_type='application/json',
                    status=404
                )

            # Check if there is an active session
            if session:
                new_consume = Consume(value=value, session=session)
                new_consume.save()


                return HttpResponse(
                    status=200,
                    content_type='application/json',
                )

            response = dict()
            response['error'] = 'no active session'
            return HttpResponse(
                json.dumps(response),
                content_type='application/json',
                status=404
            )
        response = dict()
        response['error'] = 'bad request'
        return HttpResponse(
            json.dumps(response),
            content_type='application/json',
            status=400
        )

class StateApiView(View):

    def get(self, request, *args, **kwargs):

        if 'pluckers' in kwargs:

            uuid = kwargs['pluckers']

            pluckers = Pluckers.objects.filter(uuid=uuid).first()

            response = dict()
            response['state'] = pluckers.state

            return HttpResponse(
                json.dumps(response),
                content_type='application/json',
                status=200
            )

        response = dict()
        response['error'] = 'bad request'
        return HttpResponse(
            json.dumps(response),
            content_type='application/json',
            status=400
        )

class ChangeStateApiView(View):

    def get(self, request, *args, **kwargs):

        if 'pluckers' in kwargs:

            uuid = kwargs['pluckers']

            pluckers = Pluckers.objects.filter(uuid=uuid).first()

            pluckers.state = not pluckers.state
            pluckers.save()

            return HttpResponseRedirect('/devices/' + uuid)

