from locust import HttpUser, task, between
import random


class TestUser(HttpUser):
    host = "http://localhost:1145"
    wait_time = between(1, 3)

    @task
    def login(self):
        # 模拟 POST 请求，发送登录信息
        offset = random.randint(0, 10000)
        count = random.randint(1, 10)
        self.client.post("/get_latest_news", json={"offset": offset, "count": count})


