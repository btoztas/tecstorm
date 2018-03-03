# -*- coding: utf-8 -*-
import json

from django.contrib.auth.decorators import login_required
from django.core.serializers import serialize
from django.http import HttpResponse, HttpResponseRedirect
from django.shortcuts import render
from django.utils.decorators import method_decorator
from django.views import View

from dashboard.forms import NewPluckersForm
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


class SessionApiView(View):

    def post(self, request, *args, **kwargs):

        body = json.loads(request.body)

        if 'tag' in body and 'pluckers' in kwargs:

            tag = body['tag']
            pluckers_uuid = body['pluckers']

            try:
                user_tag = UserTag.objects.get(tag=tag)
                user = user_tag.user
                pluckers = Pluckers.objects.filter(uuid=pluckers_uuid)

            except (UserTag.DoesNotExist, ValueError):

                response = dict()
                response['error'] = 'tag not found'
                return HttpResponse(
                    json.dumps(response),
                    content_type='application/json',
                    status=404
                )

            new_session = Session(user=user, pluckers=pluckers, active=True)
            new_session.save()

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

#
# class PluckersApiView(View):
#
#     def get(self, request, *args, **kwargs):
#
#         # Check if uuid is present in URL path (/api/pluckers/<uuid>)
#         if 'uuid' in kwargs:
#             uuid = kwargs['uuid']
#
#             try:
#                 pluckers = Pluckers.objects.filter(uuid=uuid).first()
#                 plugs = Plug.objects.filter(pluckers=pluckers)
#
#             except (Pluckers.DoesNotExist, ValueError):
#                 response = dict()
#                 response['error'] = 'resource not found'
#                 return HttpResponse(
#                     response,
#                     content_type='application/json',
#                     status=404
#                 )
#
#             response = dict()
#             response['pluckers'] = serialize("json", pluckers)
#             response['plugs'] = serialize("json", plugs)
#
#             return HttpResponse(
#                 json.dumps(response),
#                 content_type='application/json',
#                 status=200
#             )
