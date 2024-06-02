FROM ubuntu

WORKDIR /bin

COPY web .

COPY data/ ../var/netwatchdog/www/

ENV PORT=443

EXPOSE 443

CMD [ "/bin/web" ]