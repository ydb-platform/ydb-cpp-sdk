.PHONY: all compile run run-shell clean distclean check help

CONN ?= Driver=YDB;Endpoint=localhost:2136;Database=/local;
ERLC ?= erlc
ERL ?= erl

all: compile

help:
	@echo "YDB Series Example - Erlang ODBC"
	@echo ""
	@echo "Targets:"
	@echo "  make compile          Compile Erlang modules"
	@echo "  make run              Run the example"
	@echo "  make run CONN='...'   Run with a custom connection string"
	@echo "  make run-shell        Start Erlang shell with compiled modules"
	@echo "  make check            Check Erlang ODBC availability"
	@echo "  make clean            Remove compiled files"
	@echo ""
	@echo "Examples:"
	@echo "  make run"
	@echo "  make run CONN=\"Driver=YDB;Endpoint=myhost:2136;Database=/mydb;\""

prepare:
	@mkdir -p ebin

compile: prepare
	@echo "Compiling Erlang modules..."
	$(ERLC) -o ebin src/*.erl
	@echo "Done."

run: compile
	@echo "Running YDB Series Example..."
	$(ERL) -pa ebin -noshell -eval 'application:load(odbc), application:start(odbc), ydb_series_client:run("$(CONN)"), halt().'

run-shell: compile
	@echo "Starting Erlang shell with ydb_series_client..."
	$(ERL) -pa ebin

check:
	@echo "Checking ODBC support in Erlang..."
	@$(ERL) -noshell -eval 'application:load(odbc), io:format("~p~n", [application:start(odbc)]), halt().'

clean:
	@rm -rf ebin/*.beam
	@rm -rf *.beam
	@rm -rf erl_crash.dump

distclean: clean
	@rm -rf ebin
