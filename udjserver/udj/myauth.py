from django.contrib.auth.models import User
from django.template import RequestContext
from django.shortcuts import get_object_or_404
from django.shortcuts import render_to_response
from django.http import HttpRequest
from django.http import HttpResponse
from django.http import HttpResponseBadRequest
from django.http import HttpResponseForbidden
from django.views.decorators.csrf import csrf_exempt
import random
from models import Ticket

def validAuthRequest(request):
  if not request.method == "POST":
    return False
  elif not request.POST.__contains__("username") \
    or not request.POST.__contains__("password"):
    return False
  return True
  

def generateRandomHash():
  rand_hash = random.getrandbits(128)
  toReturn = "%032x" % rand_hash
  return toReturn

def getUniqueRandHash():
  rand_hash = generateRandomHash()
  while Ticket.objects.filter(ticket_hash=rand_hash):
    rand_hash = generateRandomHash()
  return rand_hash


def getTicketForUser(userRequestingTicket):
  currentTickets = Ticket.objects.filter(user=userRequestingTicket)
  if currentTickets:
    for ticket in currentTickets:
      ticket.delete()
  toReturn = Ticket(user=userRequestingTicket, ticket_hash=getUniqueRandHash())
  toReturn.save()
  return toReturn 

@csrf_exempt
def authenticate(request):
  if not validAuthRequest(request):
    return HttpResponseBadRequest()

  userToAuth = get_object_or_404( \
    User, username__exact=request.POST.__getitem__("username"))
  if userToAuth.check_password(request.POST.__getitem__("password")):
    ticket = getTicketForUser(userToAuth)
    response = HttpResponse()
    response['udj_ticket_hash'] = ticket.ticket_hash
    return response
  else:
    print "bad username and password"
    return HttpResponseForbidden()