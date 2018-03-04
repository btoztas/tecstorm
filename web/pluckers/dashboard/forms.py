from django import forms


class NewPluckersForm(forms.Form):
    uuid = forms.CharField(label='uuid', max_length=20)
    name = forms.CharField(label='name', max_length=50)
    location = forms.CharField(label='location', max_length=100)

class NewTagForm(forms.Form):
    uuid = forms.CharField(label='uuid', max_length=20)
    name = forms.CharField(label='name', max_length=50)
