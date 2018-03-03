from django.conf.urls import url
from django.views.decorators.csrf import csrf_exempt
from django.contrib.auth import views as auth_views
from dashboard.views import HomeView, DevicesView, NewDeviceView, DeviceView, SessionApiView, TagsView, SessionsView

urlpatterns = [

    # LOGIN/LOGOUT
    url(r'^login/$', auth_views.login, {'template_name': 'login.html'}, name='login'),
    url(r'^logout/$', auth_views.logout, {'next_page': '/login'}, name='logout'),

    # HOME
    url(r'^$', HomeView.as_view()),

    # TAGS
    url(r'^tags/$', TagsView.as_view()),

    # SESSION
    url(r'^sessions/$', SessionsView.as_view()),

    # DEVICES
    url(r'^devices/$', DevicesView.as_view()),
    url(r'^devices/new/$', NewDeviceView.as_view()),
    url(r'^devices/(?P<uuid>.*)/$', DeviceView.as_view()),

    # SESSION API
    url(r'^api/session/(?P<pluckers>.*)/$', csrf_exempt(SessionApiView.as_view())),

]
