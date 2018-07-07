import base64
import json
import requests


def detectFace(imageFile):

    url = 'https://vision.googleapis.com/v1/images:annotate?key='
    key = 'AIzaSyCbezfge-FqcRvL1fNjNmNBW7LNNw_JgLw'

    with open(imageFile, 'rb') as buffer:
        image = buffer.read()

    data = json.dumps({
        'requests': [{
            'image': {'content': base64.b64encode(image).decode('UTF-8')},
            'features': [{'type': 'FACE_DETECTION', 'maxResults': 1}]
        }]
    })

    try:

        response = requests.post(
            url=url+key, data=data, headers={'Content-Type': 'application/json'}).json()

        emotions = [(key.replace('Likelihood', ''), {'UNKNOWN': 0.0, 'VERY_UNLIKELY': 0.2, 'UNLIKELY': 0.4, 'POSSIBLE': 0.6, 'LIKELY': 0.8, 'VERY_LIKELY': 1.0}[value])
                    for key, value in response['responses'][0]['faceAnnotations'][0].items() if key in ['joyLikelihood', 'sorrowLikelihood', 'angerLikelihood', 'surpriseLikelihood']]

        return max(emotions, key=lambda emotion: emotion[1])

    except:

        return ('unknown', 0.0)
