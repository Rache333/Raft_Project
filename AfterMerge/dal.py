import redis

r = redis.StrictRedis(host='127.0.0.1',port=7777, db=0, charset='utf-8', decode_responses=True)

def editValToDB(key, value):
    #r.set(key ,value)
    print("Done\n")

def deleteValfromDB(key):
    #r.delete(key)
    print("Done\n")