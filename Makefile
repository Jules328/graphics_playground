PY_VENV := graphics_venv

.PHONY: all clean
all: $(PY_VENV)

$(PY_VENV): $(PY_VENV)/touchfile

$(PY_VENV)/touchfile: requirements.txt
	test -d $(PY_VENV) || python3 -m venv $(PY_VENV)
	. $(PY_VENV)/bin/activate; pip install -r requirements.txt
	touch $(PY_VENV)/touchfile

clean:
	rm -rf $(PY_VENV)
	find -iname "*.pyc" -delete
