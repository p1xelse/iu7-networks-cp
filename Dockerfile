FROM gcc:latest

COPY . .

RUN gcc -o ./app -I./server/include ./server/src/*.c -pthread -Wpedantic 
EXPOSE 8080

CMD ["./app"]