# VoterNN API Documentation   {#api}
This document lists the API documentation for the reference implementation of VoterNN.

# Authentication and authorization
Every POST request's JSON body must contain a `token` attribute. This token identifies the simulation. View the token in the backend UI.
If the token is not provided, or is incorrect, a `401 Unauthorized` status code is returned.

# Reference
> [!note]
> You *don't* need to manage this manually. The simulation itself decides on this. Adding editing tools is highly recommended though.

## Simulation management

### POST /map/
Gets map of districts.

District object definition:
```json
{
	"name": "<name of district>",
	"r": "<r>",
	"g": "<g>",
	"b": "<b>",
	"uuid": "<uuid>",
	"crime_rate": "<number>", 
	"gdp_capita": "<number>", 
	"minority_percentage": "<number>",
	"freedom": "<number>",
	"infrastructure": "<number>",
	"life_expectancy": "<number>",
	"price_food": "<number>",
	"price_water": "<number>",
	"price_energy": "<number>",
	"price_housing": "<number>",
	"tax_progressiveness": "<number>",
}
```

Parameters _r,g, b_ are integers in range from 0 to 255 inclusive.

Tax progressiveness if second derivative of function gotten by linear regression.

Grid Mapping object definition:

_X, Y are coordinates. X, Y >= 0; X < xs; Y < ys;_
```json
{
	"x": 1,
	"y": 1,
	"uuid": "<uuid>"
}
```

\returns **districts** Array of _district_ objects.
\returns **gridmapping** Array of _grid\_mapping_ objects. Contains
\returns **xs** X size of grid.
\returns **ys** Y size of grid.

\since 1.0.0

### POST /increment/
Increments a simulation by N days.

\param days Amount of days to increment the simulation by.

\since 1.0.0

### POST /reset/
Resets the simulation. Resets all of the voters' positions, all parties are disbanded.
Districts are **randomly regenerated**.

\since 1.0.0

### POST /snap/
Call snap elections.

\param type Type of election.

Type parameter may be:
- `parliament` - Parliamentary (lower chamber) elections only.
- `senate` - Senate (upper chamber) elections only.
- `president` - Presidential elections.
- `general` - Only parliamentary, but for both chambers.
- `all` - All elections.

\since 1.0.0

### POST /system/
Change parliamentary electoral system.

\param system Name of system.

Default values available for param:
- `d_hondt`
- `fptp`
- `hungry_orbman`
- `perfect`
- `stv`

\since 1.0.0

### POST /presidential/
Change presidential electoral system.

\param system Name of system.

Default values available for param:
- `electoral_college`
- `two_round`

\since 1.0.0

## Polling management
### POST /polling/get
Gets all polling data.

\returns **polls** Array of poll results.

\since 1.0.0
\see POST /polling/last
\see POST /polling/run

### POST /polling/last
Gets last poll.

\since 1.0.0

\returns **national** Array of results nationally.
\returns **districts** Array of districts with array of results `results`. 

### POST /polling/run
Runs a poll and returns its results.

\since 1.0.0

\returns **national** Array of results nationally.
\returns **districts** Array of districts with array of results `results`. 

> [!tip]
> Results are defined as JSON objects with `party_uuid` and `percentage` items.

## Voter management

### POST /voter/get
Gets information about voter.
\param voter_uuid Voter UUID.

\returns **name** First name of voter.
\returns **middlename** Middle name of voter. Leave empty if none.
\returns **surname** Surname of voter.
\returns **security** Security opinion position.
\returns **risk_aversion** Risk aversion opinion position.
\returns **public_spending** Public spending support opinion position.
\returns **cost_of_living** Cost of living priority opinion position.
\returns **minorities_support** Minority and multiculturalism support opinion position.
\returns **stability** National stability opinion position.
\returns **job_security** Citizens' job security opinion position.
\returns **future_outlook** Future outlook opinion position.
\returns **equality_markets** _Equality vs markets_ opinion position.
\returns **nation_globe** _National or globalized_ opinion position.
\returns **liberty_authority** _Liberty vs authority_ opinion position.
\returns **tradition_progress** _Tradition vs progress_ opinion position.

\since 1.0.0
\see opinion_position

### POST /voter/create
Creates a new voter.

\param name First name of voter.
\param middlename Middle name of voter. Leave empty if none.
\param surname Surname of voter.
\param age Age of voter.
\param security Security opinion position.
\param risk_aversion Risk aversion opinion position.
\param public_spending Public spending support opinion position.
\param cost_of_living Cost of living priority opinion position.
\param minorities_support Minority and multiculturalism support opinion position.
\param stability National stability opinion position.
\param job_security Citizens' job security opinion position.
\param future_outlook Future outlook opinion position.
\param equality_markets _Equality vs markets_ opinion position.
\param nation_globe _National or globalized_ opinion position.
\param liberty_authority _Liberty vs authority_ opinion position.
\param tradition_progress _Tradition vs progress_ opinion position.

\since 1.0.0
\see opinion_position

### POST /voter/edit
Edits an existing voter.

\param voter_uuid Voter UUID.
\param name First name of voter.
\param middlename Middle name of voter. Leave empty if none.
\param surname Surname of voter.
\param age Age of voter.
\param security Security opinion position.
\param risk_aversion Risk aversion opinion position.
\param public_spending Public spending support opinion position.
\param cost_of_living Cost of living priority opinion position.
\param minorities_support Minority and multiculturalism support opinion position.
\param stability National stability opinion position.
\param job_security Citizens' job security opinion position.
\param future_outlook Future outlook opinion position.
\param equality_markets _Equality vs markets_ opinion position.
\param nation_globe _National or globalized_ opinion position.
\param liberty_authority _Liberty vs authority_ opinion position.
\param tradition_progress _Tradition vs progress_ opinion position.

\since 1.0.0
\see opinion_position

### POST /voter/delete
Deletes a voter. _Think of it as exile._
\param voter_uuid Voter UUID.

\since 1.0.0

## District management

### POST /district/get
Gets information about district.

\since 1.0.0

### POST /district/create
Creates a new district.

\param name Unofficial district name.

\since 1.0.0

### POST /district/edit
Edits a districts' values.

\since 1.0.0

### POST /district/delete
Removes a district. Voters stay. District's territory is randomly assigned to neighbouing districts.

\since 1.0.0

### POST /district/nuke
Removes a district and all of its voters.

\since 1.0.0

### POST /district/merge
Merges two districts together.

\since 1.0.0

### POST /district/reassign
Reassigns a grid tile to a district.

\since 1.0.0

## Party management
### POST /party/create
Manually create a new party.

\param name Name of party
\param security Security opinion position.
\param risk_aversion Risk aversion opinion position.
\param public_spending Public spending support opinion position.
\param cost_of_living Cost of living priority opinion position.
\param minorities_support Minority and multiculturalism support opinion position.
\param stability National stability opinion position.
\param job_security Citizens' job security opinion position.
\param future_outlook Future outlook opinion position.
\param equality_markets _Equality vs markets_ opinion position.
\param nation_globe _National or globalized_ opinion position.
\param liberty_authority _Liberty vs authority_ opinion position.
\param tradition_progress _Tradition vs progress_ opinion position.

\returns `party_uuid` New party UUID.

\since 1.0.0
\see opinion_position

### POST /party/edit

\since 1.0.0
\see opinion_position

### POST /party/delete
Delete (ban) an existing party.
\param party_uuid Party UUID.

> [!note]
> Deleting a party does not remove its supporters. A new party may be formed by them.
> **This correctly models reality and will not be changed.**

\since 1.0.0
\see opinion_position

### POST /party/merge
Merge 2 parties. May lead to a splinter in the near future, unless option banned.

\param party_uuid_1 First party UUID.
\param party_uuid_2 Second party UUID.
\param name New party name. If name empty, name of the larger party (by amount of MPs, then by party members) used.

## Company management
### POST /company/add

\since 1.0.0

### POST /company/edit

\since 1.0.0

### POST /company/delete

\since 1.0.0

## Product management

### POST /product/add

### POST /product/edit

### POST /product/delete

## Statistics management

### POST /statistics/edit/gdp

\since 1.0.0

## Crises management

### POST /crisis/create/

\since 1.0.0

### POST /crisis/delete/

\since 1.0.0

## Media management
### POST /media/create/
Manually create a media provider.

\param name Name of media provider.

\since 1.0.0

### POST /media/edit/

\since 1.0.0

### POST /media/delete/

\since 1.0.0

## Law management
### POST /law/add/
Add manually written law. The law will have to be voted on.

\param text Text, as string in JSON object, with the full text of the law, with newlines.

> [!note]
> Manually written laws do not have any negative modifiers from "being from a different party". They will be considered by all parties.

\since 1.0.0

### POST /law/force/
Add manually written law. The law will **not** be voted on and will passed automatically. It may be repealed.

### POST /law/repeal/
Force repeal some law. This does not block the parliament from drafting a similar one in the future.

\param law_uuid UUID of law.

\since 1.0.0

### POST /law/article/add
Adds an article to a law.

\param law_uuid UUID of law.
\param text Text of article.

\since 1.0.0

### POST /law/article/repeal
Repeal an article of a law.

\param law_uuid UUID of law.
\param article Article number.

\since 1.0.0

## Constitution management
### POST /constitution/article/add/

### POST /constitution/article/remove/

### POST /constitution/redraft/


## Miscellaneous
### GET /version/
Gets the version and build number as JSON.
Build number of the next version is always larger than the previous.
Is the major build number is the same, the client and server are **always** compatible. Is the major build number is different, the request will **always** be refused.
Minor versions may add new features, only check if you depend on the new functionality.
Build numbers can be completely different.

\returns version_string Version string and name of backend. Example: `VoterNN Reference 1.0.0`
\returns major Major version. Every change of this version means that the **change is breaking.**
\returns minor Minor version. Usually indicates some new features.
\returns build Bugfix or patch version.

> [!note]
> Since this is a non-authenticated API route, different rate limits apply here.

\since 1.0.0

# Success
### 200 OK
Returned for all GET requests on success.
### 204 No Content
Returned for requests that delete or modify something on the server, after POSTs.

# Errors
### 400 Bad Request
Passed data is wrong, malformed or simply nonsensical.

### 401 Unauthorized
Your token is incorrect and does not reference any simulation on the instance.

### 403 Forbidden
Your IP address has been banned from accessing the particular VoterNN instance.

> [!important]
> Only the administrator of the backend can remove your IP from the blocklist.

> [!note]
> The blocklist is per instance, not per simulation! 
> Creating a new simulation on the same instance will not remove the block.

### 404 Not Found
Path that is being requested does not exist.

### 405 Method Not Allowed
An incorrect method has been used. Most requests are POST, some are GET. Other methods are not allowed.

> [!note]
> The server will return an `Allow` header with the correct request.

### 429 Too Many Requests
Returned when rate limit has been tripped for the current IP address. Wait at least 10 seconds before sending any new request.

> [!tip]
> It is recommended to disable the UI and to add a countdown while this timeout is in effect.

### 500 Internal Server Error
This error code is returned when a handler in the server throws and exception.

### 503 Service Unavailable
This error code is returned when the server instance is running, however, no simulations have been enabled on it yet.

### 505 HTTP Version Not Supported
VoterNN only supports HTTP/1.1. Other versions specified in the request header will return this code.
