def getDjangoTicketHeader():
  return "HTTP_X_UDJ_TICKET_HASH"

def getTicketHeader():
  return "X-Udj-Ticket-Hash"

def getUserIdHeader():
  return "X-Udj-User-Id"

def getGoneResourceHeader():
  return "X-Udj-Gone-Resource"

def getUUIDHeader():
  return "X-UDJ-Machine-UUID"

def getDjangoUUIDHeader():
  return "HTTP_X_UDJ_MACHINE_UUID"

def getApiVersionHeader():
  return "X-Udj-Api-Version"

def getDjangoApiVersionHeader():
  return "HTTP_X_UDJ_API_VERSION"
