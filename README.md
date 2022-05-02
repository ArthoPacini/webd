# webd
Decentralized Web 

This software enables the delivery of static websites in a decentralized manner, through a decentralized network of nodes serving files. Users may upload and update websites


## How to use

Install required libraries and packages by running ```sudo apt install libboost-all-dev libssl clang gcc make build-essentials ``` and
compile the software by running ```make```


The executable can now be opened, here are some arguments you may pass into it:

1. -p {port_number}
2. -n {node_name}

The default port number is ```8080```, the default node name is ```WebD_Node```.

Usage example: ```./main -p 8082 -n Testing_Node ``` will execute the node using port 8082 and name 'Testing_Node'

## Websites out of the box

1 search.webd
2 cat.com
3 teste.com
Run your node and type in the browser http://localhost:PORT/api/v1/site/cat.com or http://localhost:PORT/api/v1/site/search.webd 
Replace PORT by the port you are using

## Node HTTP API

### GET /api/v1/ping
Will reply with a JSON response 

```json
{ 
"pong":true 
}
```

### GET /api/v1/site/{site_name}
Will try to deliver you with the website, will deliver a 404 not found web page otherwise, example:

#### GET /api/v1/site/cat.com
```html 
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CAT SITE</title>
</head>
<body>
    Decentralized cat site!<br>
    <img src="./cat.com/cat.jpg" alt="">
</body>
</html>
```


### GET /api/v1/resume
Returns the Node sites index

```json
[
    {
        "description": "Test Site",
        "domain": "teste.com",
        "files": [
            "teste.com/index.html"
        ],
        "version": 1
    },
    {
        "description": "Not Found 404 Default",
        "domain": "notfound",
        "files": [
            "notfound/index.html"
        ],
        "version": 1
    },
    {
        "description": "Cat Site",
        "domain": "cat.com",
        "files": [
            "cat.com/cat.jpg",
            "cat.com/index.html"
        ],
        "version": 1
    },
    {
        "description": "WebD Search",
        "domain": "search.webd",
        "files": [
            "search.webd/index.html",
            "search.webd/style.css",
            "search.webd/main.js"
        ],
        "version": 1
    }
]

```

### GET /api/v1/known-nodes
Returns a list of known nodes


```json
[
    {
        "ip": "localhost",
        "name": "WebD_Node",
        "port": "8086"
    },
    {
        "ip": "localhost",
        "name": "LetsGoWebD",
        "port": "8085"
    }
]
```

### POST /api/v1/known-nodes
Returns a list of known nodes, you may post a json body containing information about your node, so the receiving node adds your address to their known nodes database. Example:

POST BODY:

```json
{
"name":"NewNodeInTown",
"ip":"123.123.123.123",
"port":"5343"
}

```

Response:


```json
[
    {
        "ip": "localhost",
        "name": "WebD_Node",
        "port": "8086"
    },
    {
        "ip": "localhost",
        "name": "LetsGoWebD",
        "port": "8085"
    },
    {
        "name":"NewNodeInTown",
        "ip":"123.123.123.123",
        "port":"5343"
    }
]

```
