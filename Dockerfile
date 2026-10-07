FROM --platform=linux/amd64 python:3.14-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

CMD ["bash"]

# docker build -t my-env . && docker run --rm -it -v "$(pwd):/app" my-env