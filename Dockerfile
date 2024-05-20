FROM ubuntu

WORKDIR /bin

COPY . .

ENV PORT=8080

EXPOSE 8080

CMD [ "/bin/web" ]